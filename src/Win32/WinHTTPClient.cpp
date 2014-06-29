/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#include "stdafx.h"
#include "Win32/WinHTTPClient.h"
#include "Win32/ErrorToString.h"

// this is an IHTTPClient implementation using WinHTTP.
// ( http://msdn.microsoft.com/en-us/library/windows/desktop/aa384270%28v=vs.85%29.aspx )

// some helpers:

namespace WinHttpHelpers
{
	static std::wstring Utf8ToWide(const std::string& utf8)
	{
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().from_bytes(utf8);
	}

	static bool QueryOptionDword(HINTERNET hInternet, DWORD dwOption, DWORD& result)
	{
		DWORD size = sizeof(DWORD);

		return ::WinHttpQueryOption(hInternet, dwOption, &result, &size) != FALSE;
	}

	static bool SetOptionDword(HINTERNET hInternet, DWORD dwOption, DWORD dwValue)
	{
		return ::WinHttpSetOption(hInternet, dwOption, &dwValue, sizeof(DWORD)) != FALSE;
	}

	static bool QueryHeadersInt(HINTERNET hRequest, DWORD dwOption, int32_t& result)
	{
		DWORD codeSize = sizeof(result);

		return ::WinHttpQueryHeaders(hRequest, dwOption | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX, &result, &codeSize, WINHTTP_NO_HEADER_INDEX) != FALSE;
	}

	static bool QueryHeadersString(HINTERNET hRequest, DWORD dwOption, std::wstring& result)
	{
		DWORD bufSize = 0;

		::SetLastError(0);
		::WinHttpQueryHeaders(hRequest, dwOption, WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &bufSize, WINHTTP_NO_HEADER_INDEX);

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			// use vector because it's guaranteed to be continuous in memory.
			std::vector<wchar_t> buf(bufSize / sizeof(wchar_t) + 1);

			if (::WinHttpQueryHeaders(hRequest, dwOption, WINHTTP_HEADER_NAME_BY_INDEX, buf.data(), &bufSize, WINHTTP_NO_HEADER_INDEX))
			{
				result = std::wstring(buf.begin(), buf.end());

				return true;
			}
		}

		return false;
	}

	static const wchar_t* SecureErrorToString(DWORD tlsErrorCode)
	{
		if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED))
			return L"Certification revocation checking has been enabled, but the revocation check failed to verify whether a certificate has been revoked.";
		else if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT))
			return L"SSL/TLS certificate is invalid.";
		else if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED))
			return L"SSL/TLS certificate was revoked.";
		else if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA))
			return L"Windows is unfamiliar with the Certificate Authority that generated the server's certificate.";
		else if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID))
			return L"SSL/TLS certificate common name (host name field) is incorrect.";
		else if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID))
			return L"SSL/TLS certificate date that was received from the server is bad. The certificate is expired or not yet valid.";
		else if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_CERT_WRONG_USAGE))
			return L"SSL/TLS certificate's permitted usage flag does not match.";
		else if (0 != (tlsErrorCode & WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR))
			return L"An internal SSL/TLS protocol error has occured. Please check your connection settings.";
		else if (tlsErrorCode)
		{
			static __declspec(thread) wchar_t buf[64];
			swprintf_s(buf, 64, L"Unknown SSL/TLS error %08X (%d)!", tlsErrorCode, tlsErrorCode);
			return buf;
		}
		else
			return L"Success.";
	}
}

// actual implementation:

using namespace Player95::Win32;

WinHTTPClient::WinHTTPClient() :
	m_hSession(0), m_allowInsecureTLS(false)
{
}

bool WinHTTPClient::Init()
{
	// :TODO: proxy support

	m_hSession = ::WinHttpOpen(L"",
		WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS,
		WINHTTP_FLAG_ASYNC);

	if (!m_hSession)
		return false;

	// go async:
	
	DWORD_PTR ctx = reinterpret_cast<DWORD_PTR>(this);
	if (!::WinHttpSetOption(m_hSession, WINHTTP_OPTION_CONTEXT_VALUE, &ctx, sizeof(ctx)))
		return false;
	
	if (::WinHttpSetStatusCallback(m_hSession, WinHttpCallback, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_SECURE_FAILURE, 0) == WINHTTP_INVALID_STATUS_CALLBACK)
		return false;

	::WinHttpSetTimeouts(m_hSession,
		20 * 1000, // DNS
		10 * 1000, // Connect
		10 * 1000, // Send
		20 * 1000 // Receive
	);

	return true;
}

void WinHTTPClient::UseAuthentication(const std::string& username, const std::string& password)
{
	m_authUsername = WinHttpHelpers::Utf8ToWide(username);
	m_authPassword = WinHttpHelpers::Utf8ToWide(password);
}

bool WinHTTPClient::GetText(const std::string& url, const std::string& postData, std::function<void(int httpStatusCode, const std::string& responseBody)> callback)
{
	PRequest request;

	if (!CreateRequest(url, request))
		return false;

	request->SetVerb(postData.empty() ? HTTP_GET : HTTP_POST);
	request->SetPostData(postData);

	request->SetCompletionHandler([this, request, callback]()
	{
		// erase from map:
		{
			std::lock_guard<std::mutex> lock(m_access);
			m_activeRequests.erase(request->GetHandle());
		}

		std::wstring statusText = request->GetStatusText();
		callback(request->GetStatusCode(), request->ExtractReadBuffer());

		// request should be destroyed now.
	});

	if (!request->SendRequest())
		return false;

	std::lock_guard<std::mutex> lock(m_access);

	m_activeRequests[request->GetHandle()] = request;

	return true;
}

bool WinHTTPClient::CreateRequest(const std::string& url, WinHTTPClient::PRequest& request)
{
	URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS), 0 };

	urlComp.dwSchemeLength = (DWORD)-1;
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;

	const std::wstring wurl = WinHttpHelpers::Utf8ToWide(url);

	if (!::WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp))
		return false;

	// oh my god...
	bool isSchemeHttps = urlComp.dwSchemeLength == 5 && !_wcsnicmp(wurl.c_str(), L"https://", std::min(url.length(), size_t(8)));
	bool isSchemeHttp = !isSchemeHttps && urlComp.dwSchemeLength == 4 && !_wcsnicmp(wurl.c_str(), L"http://", std::min(url.length(), size_t(7)));

	std::wstring hostname(urlComp.lpszHostName, urlComp.dwHostNameLength);
	unsigned short port = (urlComp.nPort > 0 ? urlComp.nPort : (
		isSchemeHttps ? INTERNET_DEFAULT_HTTPS_PORT : (
		isSchemeHttp ? INTERNET_DEFAULT_HTTP_PORT : 0)));

	if (hostname.empty() || port == 0)
		return false;

	HINTERNET hConnect = 0;

	// find existing HINTERNET for host + port:
	{
		std::lock_guard<std::mutex> lock(m_access);

		auto f1 = m_hHostConnects.find(hostname);

		if (f1 != m_hHostConnects.end())
		{
			auto f2 = f1->second.find(port);

			if (f2 != f1->second.end())
			{
				hConnect = f2->second;
			}
		}
	}

	if (hConnect != 0)
	{
		DWORD type = 0;

		if (!WinHttpHelpers::QueryOptionDword(hConnect, WINHTTP_OPTION_HANDLE_TYPE, type) || type != WINHTTP_HANDLE_TYPE_CONNECT)
		{
			hConnect = 0;
		}
	}

	if (hConnect == 0)
	{
		hConnect = ::WinHttpConnect(m_hSession, hostname.c_str(), port, 0);

		if (!hConnect)
			return false;

		std::lock_guard<std::mutex> lock(m_access);

		m_hHostConnects[hostname][port] = hConnect;
	}

	request = PRequest(new Request(m_hSession, hConnect));

	request->SetPath(std::wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength));
	request->SetTLS(isSchemeHttps ? (m_allowInsecureTLS ? TLS_ALLOW_INSECURE : TLS_ON) : TLS_OFF);

	// request->SetServer(hostname, port); // not used

	if (!m_authPassword.empty())
	{
		request->SetAuth(m_authUsername, m_authPassword);
	}

	return true;
}

bool WinHTTPClient::FindRequest(HINTERNET hRequest, PRequest& request)
{
	std::lock_guard<std::mutex> lock(m_access);

	auto it = m_activeRequests.find(hRequest);

	if (it != m_activeRequests.end())
	{
		request = it->second;

		return true;
	}

	return false;
}

void CALLBACK WinHTTPClient::WinHttpCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	DWORD handleType = 0;

	if (!WinHttpHelpers::QueryOptionDword(hInternet, WINHTTP_OPTION_HANDLE_TYPE, handleType))
		return;

	WinHTTPClient *pClient = reinterpret_cast<WinHTTPClient*>(dwContext);

	if (!pClient)
		return;

	if (handleType == WINHTTP_HANDLE_TYPE_REQUEST)
	{
		PRequest request;

		if (!pClient->FindRequest(hInternet, request))
			return;

		switch (dwInternetStatus)
		{
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
			const WINHTTP_ASYNC_RESULT* error = reinterpret_cast<WINHTTP_ASYNC_RESULT*>(lpvStatusInformation);
			request->OnNetworkError(error->dwError, Win32::Helpers::ErrorToString(error->dwError, L"winhttp.dll"));
			break; }

		case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE: {
			DWORD tlsError = *reinterpret_cast<DWORD*>(lpvStatusInformation);
			request->OnNetworkError(tlsError, WinHttpHelpers::SecureErrorToString(tlsError));
			break; }

		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			request->OnRequestSent();
			break;

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			request->OnHeadersComplete();
			break;

		case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
			request->OnReadData(*reinterpret_cast<DWORD*>(lpvStatusInformation));
			break;

		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			if (dwStatusInformationLength == 0)
			{
				request->OnComplete();
			}
			break;
		default:
			_ASSERT(false);
		}
	}
	else
	{
		_ASSERT(false);
	}
}

WinHTTPClient::~WinHTTPClient()
{
	while (true)
	{
		PRequest request;

		// extract one request:
		{
			std::lock_guard<std::mutex> lock(m_access);

			if (m_activeRequests.empty())
				break;

			auto it = m_activeRequests.begin();
			request = it->second;
			m_activeRequests.erase(it);
		}

		// m_access MUST be unlocked before Cancel is invoked

		request->Cancel();
	}

	m_access.lock();

	for (auto it : m_hHostConnects)
	{
		for (auto jt : it.second)
		{
			::WinHttpCloseHandle(jt.second);
		}
	}

	m_hHostConnects.clear();

	if (m_hSession)
	{
		::WinHttpCloseHandle(m_hSession);
	}

	m_access.unlock();
}

//
// -- Request class implementation --
//

WinHTTPClient::Request::Request(HINTERNET hSession, HINTERNET hConnect) :
	m_hConnect(hConnect), m_hRequest(0), m_ownsHConnect(hConnect == 0),
	m_tls(TLS_OFF), m_verb(HTTP_GET), m_canceled(false), m_lastErrorCode(0)
{

}

bool WinHTTPClient::Request::SendRequest()
{
	if (!m_completionHandler)
		return false;

	if (m_hRequest)
		return false;

	if (!m_hConnect)
		return false;

	if (m_verb == HTTP_POST && m_postData.size() > std::numeric_limits<DWORD>::max())
		return false;

	m_hRequest = ::WinHttpOpenRequest(m_hConnect,
		m_verb == HTTP_GET ? L"GET" : L"POST",
		m_path.c_str(),
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_REFRESH | (m_tls != TLS_OFF ? WINHTTP_FLAG_SECURE : 0)
	);

	if (!m_hRequest)
		return false;

	if (!m_password.empty())
	{
		if (!::WinHttpSetCredentials(m_hRequest, WINHTTP_AUTH_TARGET_SERVER,
			WINHTTP_AUTH_SCHEME_BASIC, // only basic auth so far...
			m_username.c_str(), m_password.c_str(), nullptr))
		{
			return false;
		}
	}

	if (m_verb == HTTP_POST)
	{
		::WinHttpAddRequestHeaders(m_hRequest, L"Content-Type: application/x-www-form-urlencoded", -1, WINHTTP_ADDREQ_FLAG_REPLACE | WINHTTP_ADDREQ_FLAG_ADD);
	}

	// we do not endorse redirects:
	WinHttpHelpers::SetOptionDword(m_hRequest, WINHTTP_OPTION_REDIRECT_POLICY, WINHTTP_OPTION_REDIRECT_POLICY_NEVER);

	if (m_tls == TLS_ALLOW_INSECURE)
	{
		WinHttpHelpers::SetOptionDword(m_hRequest, WINHTTP_OPTION_SECURITY_FLAGS,
			SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID);
	}

	// note: WinHttp does not support decompressing gzip responses :-(

	if (!::WinHttpSendRequest(m_hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		m_verb == HTTP_GET ? WINHTTP_NO_REQUEST_DATA : (void*)m_postData.c_str(), (DWORD)m_postData.size(), (DWORD)m_postData.size(), NULL))
	{
		return false;
	}

	m_readBuffer.clear();

	return true;
}

void WinHTTPClient::Request::OnRequestSent()
{
	// tell WinHttp to begin receiving:
	if (!::WinHttpReceiveResponse(m_hRequest, nullptr))
		InternalError();
}

void WinHTTPClient::Request::OnHeadersComplete()
{
	// tell WinHttp to begin reading the response body:
	if (!::WinHttpQueryDataAvailable(m_hRequest, nullptr))
		InternalError();
}

void WinHTTPClient::Request::OnReadData(DWORD data_length)
{
	// :TODO: other read modes
	// :TODO: limit max. response size for in-memory buffer
	DWORD read;

	::SetLastError(0);

	if (data_length > 0)
	{
		std::vector<char> temp(data_length);

		if (!::WinHttpReadData(m_hRequest, temp.data(), data_length, &read))
			return InternalError();

		if (read != data_length)
			return InternalError();

		m_readBuffer.append(temp.begin(), temp.begin() + data_length);

		if (!::WinHttpQueryDataAvailable(m_hRequest, nullptr))
			return InternalError();
	}
	else
	{
		// trigger WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
		if (!::WinHttpReadData(m_hRequest, NULL, 0, &read))
			return InternalError();
	}
}

void WinHTTPClient::Request::OnNetworkError(DWORD code, const std::wstring& description)
{
	m_lastErrorCode = code;
	m_lastErrorDescription = description;

	InvokeCompletionHandler();
}

void WinHTTPClient::Request::OnComplete()
{
	InvokeCompletionHandler();
}

void WinHTTPClient::Request::InvokeCompletionHandler()
{
	std::function<void()> callback = m_completionHandler;

	// make sure it's never invoked again.
	// this is a guarantee to users of this class that we MUST fulfill.

	m_completionHandler = std::function<void()>();

	_ASSERT(callback);

	if (callback)
	{
		callback();
	}

	::WinHttpSetStatusCallback(m_hRequest, nullptr, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);

	// handles are closed by the destructor.
}

int WinHTTPClient::Request::GetStatusCode() const
{
	int32_t code = 0;

	WinHttpHelpers::QueryHeadersInt(m_hRequest, WINHTTP_QUERY_STATUS_CODE, code);

	return code;
}

const std::wstring WinHTTPClient::Request::GetStatusText() const
{
	std::wstring status;

	WinHttpHelpers::QueryHeadersString(m_hRequest, WINHTTP_QUERY_STATUS_TEXT, status);

	return status;
}

void WinHTTPClient::Request::Cancel()
{
	m_canceled = true;

	// :TODO: this is not 100% threadsafe:
	InvokeCompletionHandler();

	if (m_hRequest)
	{
		::WinHttpCloseHandle(m_hRequest);
		m_hRequest = 0;
	}

	m_lastErrorCode = -1;
	m_lastErrorDescription = L"Request has been canceled!";
}

void WinHTTPClient::Request::InternalError()
{
	DWORD errorCode = ::GetLastError();
	
	if (!errorCode)
		errorCode = ERROR_WINHTTP_INTERNAL_ERROR;

	OnNetworkError(errorCode, Win32::Helpers::ErrorToString(errorCode, L"winhttp.dll"));
}

WinHTTPClient::Request::~Request()
{
	if (m_hRequest)
	{
		::WinHttpCloseHandle(m_hRequest);
	}

	if (m_hConnect && m_ownsHConnect)
	{
		::WinHttpCloseHandle(m_hConnect);
	}
}

#pragma comment(lib, "Winhttp.lib")
