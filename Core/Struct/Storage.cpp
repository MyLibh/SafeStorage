// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Storage.hpp"
#include "Entry.hpp"
#include "../Crypto/Obfuscator.hpp"
#include "../Stream/GZip.hpp"
#include "../Stream/Hashed.hpp"

#include <fstream>

#include <QJsonDocument>
#include <QJsonObject>

namespace core {

Storage::Storage()
	: m_passwords(std::make_shared<PasswordsEntry>())
{
}

void Storage::AddLayer(Layer&& layer)
{ 
	m_layers.push_back(std::move(layer));

	if (layer.is_last)
		m_last_layer_decrypted = true;
}

bool Storage::UnlockLayer(std::string&& password)
{
	auto& layer = m_layers.back();
	layer.key.SetPassword(std::move(password));
	ExplicitBZero(password.data(), password.length());

	if (!layer.Decrypt(m_read_stream))
		return false;

	if (layer.is_last)
		DecryptLastLayer(m_read_stream, layer);
	else
		ReadNextLayerHeaders();

	return true;
}

void Storage::Clear() noexcept
{
	m_read_stream.clear();
	m_path.clear();
	m_layers.clear();
	m_passwords = std::make_shared<PasswordsEntry>();
	m_last_layer_decrypted = false;
}

bool Storage::SaveToFile(const std::string& path /*= ""*/)
{
	if (!path.empty())
		m_path = path;

	std::ofstream out(m_path, std::ios::binary);
	if (!out.is_open())
		return false;

	out.write(reinterpret_cast<const char*>(&SSDB_FILE_SIGNATURE), sizeof(uint32_t));

	std::stringstream stream;
	for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it)
		if (it->is_last)
			EncryptLastLayer(stream, *it);
		else
			it->Encrypt(stream);

	out << stream.rdbuf();

	return true;
}

bool Storage::LoadFromFile(const std::string& path)
{
	if (!path.empty())
		m_path = path;

	std::ifstream in(m_path, std::ios::binary);
	if (!in.is_open())
		return false;

	{
		uint32_t sign{};
		in.read(reinterpret_cast<char*>(&sign), sizeof(uint32_t));
		if (sign != SSDB_FILE_SIGNATURE)
			return false;
	}

	m_read_stream << in.rdbuf();

	ReadNextLayerHeaders();

	return true;
}

void Storage::ReadNextLayerHeaders()
{
	m_layers.push_back({});
	m_layers.back().ReadHeaders(m_read_stream);
}

void Storage::EncryptLastLayer(std::stringstream& dst, const Layer& layer)
{
	layer.WriteHeaders(dst);

	std::stringstream content_stream;
	const auto headers_hash = layer.ComputeHeadersHash();
	content_stream.write(reinterpret_cast<const char*>(headers_hash.data()), headers_hash.size());
	content_stream.write(reinterpret_cast<const char*>(layer.stream_start_bytes.data()), layer.stream_start_bytes.size());

	hashed_ostreambuf hashed_streambuf(content_stream);
	std::ostream hashed_stream(&hashed_streambuf);
	if (Obfuscator obfuscator(layer.GetInnerRandomStream()); layer.compression_type == CompressionType::GZip)
	{
		gzip_ostreambuf gzip_streambuf(hashed_stream);
		std::ostream gzip_stream(&gzip_streambuf);

		WriteJSON(gzip_stream, obfuscator);
		gzip_stream.flush();
	}
	else
		WriteJSON(hashed_stream, obfuscator);

	hashed_stream.flush();

	auto cipher = layer.GetCipher();
	if (!cipher)
		return;

	cipher->Encrypt(content_stream, dst);
}

void Storage::DecryptLastLayer(std::stringstream& src, const Layer& layer)
{
	hashed_istreambuf hashed_streambuf(src);
	std::istream hashed_stream(&hashed_streambuf);
	if (Obfuscator obfuscator(layer.GetInnerRandomStream()); layer.compression_type == CompressionType::GZip)
	{
		gzip_istreambuf gzip_streambuf(hashed_stream);
		std::istream gzip_stream(&gzip_streambuf);

		ReadJSON(gzip_stream, obfuscator);
	}	
	else
		ReadJSON(hashed_stream, obfuscator);

	m_last_layer_decrypted = true;
}

void Storage::WriteJSON(std::ostream& dst, Obfuscator& obfuscator)
{
	if (!m_passwords)
		return;

	QJsonObject obj;
	m_passwords->ToJson(obj, obfuscator);

	QJsonDocument doc(obj);
	dst << QString(doc.toJson(QJsonDocument::Compact)).toStdString();
}

void Storage::ReadJSON(std::istream& src, Obfuscator& obfuscator)
{
	QJsonDocument doc;
	{
		std::string str(std::istreambuf_iterator<char>(src), {});
		doc = QJsonDocument::fromJson(str.c_str());
	}

	m_passwords = std::make_shared<PasswordsEntry>(doc["passwords"], obfuscator);
	//m_fs = doc["fs"];
}

} // namespace core