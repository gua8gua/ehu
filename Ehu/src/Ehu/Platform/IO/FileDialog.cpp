#include "ehupch.h"
#include "FileDialog.h"

#if defined(_WIN32) || defined(EHU_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <shobjidl.h>
#endif

namespace Ehu {

#if defined(_WIN32) || defined(EHU_PLATFORM_WINDOWS)
	static std::string WideToUtf8(const wchar_t* wstr) {
		if (!wstr || !*wstr) return {};
		int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
		if (len <= 0) return {};
		std::string out(len - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &out[0], len, nullptr, nullptr);
		return out;
	}
#endif

	bool FileDialog::OpenFolder(const std::string& title, std::string& outPath) {
#if defined(_WIN32) || defined(EHU_PLATFORM_WINDOWS)
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		IFileDialog* pfd = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
		if (FAILED(hr) || !pfd) return false;

		DWORD opts = 0;
		if (SUCCEEDED(pfd->GetOptions(&opts)))
			pfd->SetOptions(opts | FOS_PICKFOLDERS);

		hr = pfd->Show(nullptr);
		bool ok = false;
		if (SUCCEEDED(hr)) {
			IShellItem* psi = nullptr;
			if (SUCCEEDED(pfd->GetResult(&psi)) && psi) {
				PWSTR pszPath = nullptr;
				if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
					outPath = WideToUtf8(pszPath);
					ok = true;
					CoTaskMemFree(pszPath);
				}
				psi->Release();
			}
		}
		pfd->Release();
		return ok;
#else
		(void)title;
		(void)outPath;
		return false;
#endif
	}

	static std::wstring Utf8ToWide(const std::string& str) {
		if (str.empty()) return {};
		int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
		if (len <= 0) return {};
		std::wstring out(len, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &out[0], len);
		return out;
	}

	bool FileDialog::OpenFile(const std::string& title, const std::string& filterName, const std::string& filterSpec, std::string& outPath) {
#if defined(_WIN32) || defined(EHU_PLATFORM_WINDOWS)
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		IFileDialog* pfd = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
		if (FAILED(hr) || !pfd) return false;

		std::wstring wName = Utf8ToWide(filterName);
		std::wstring wSpec = Utf8ToWide(filterSpec);
		COMDLG_FILTERSPEC spec = { wName.c_str(), wSpec.c_str() };
		pfd->SetFileTypes(1, &spec);

		hr = pfd->Show(nullptr);
		bool ok = false;
		if (SUCCEEDED(hr)) {
			IShellItem* psi = nullptr;
			if (SUCCEEDED(pfd->GetResult(&psi)) && psi) {
				PWSTR pszPath = nullptr;
				if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
					outPath = WideToUtf8(pszPath);
					ok = true;
					CoTaskMemFree(pszPath);
				}
				psi->Release();
			}
		}
		pfd->Release();
		return ok;
#else
		(void)title;
		(void)filterName;
		(void)filterSpec;
		(void)outPath;
		return false;
#endif
	}

} // namespace Ehu
