#pragma once

#include "Layer.hpp"

#include <filesystem>
#include <sstream>
#include <vector>

namespace core {

struct PasswordsEntry;
class Obfuscator;

class Storage
{
public:
	Storage();

	template<typename... Args>
	void AddHeader(Args&&... args) { m_headers.emplace(std::forward<Args>(args)...); }

	[[nodiscard]] std::shared_ptr<PasswordsEntry> GetPasswords() const noexcept { return m_passwords; }

	[[nodiscard]] bool IsLastLayerDecrypted() const noexcept { return m_last_layer_decrypted; }
	[[nodiscard]] size_t GetLayersNumber() const noexcept { return m_layers.size(); }
	void AddLayer(Layer&& layer);
	bool UnlockLayer(std::string&& password);

	void Clear() noexcept;

	bool SaveToFile(const std::string& path = "");
	bool LoadFromFile(const std::string& path);

	[[nodiscard]] const std::filesystem::path& GetPath() const noexcept { return m_path; }

private:
	void ReadNextLayerHeaders();

	void EncryptLastLayer(std::stringstream& dst, const Layer& layer);
	void DecryptLastLayer(std::stringstream& src, const Layer& layer);

	void ReadJSON(std::istream& src, Obfuscator& obfuscator);
	void WriteJSON(std::ostream& dst, Obfuscator& obfuscator);

private:
	std::stringstream m_read_stream;
	std::filesystem::path m_path;
	std::vector<Layer> m_layers;
	std::shared_ptr<PasswordsEntry> m_passwords;
	bool m_last_layer_decrypted{};
};

} // namespace core