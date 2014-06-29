/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#ifndef _WIN_HTTP_CLIENT_H
#define _WIN_HTTP_CLIENT_H

#include "Common/IHTTPClient.h"
#include "Winhttp.h"

namespace Player95
{
	namespace Win32
	{
		class WinHTTPClient : public Common::IHTTPClient
		{
		public:
			WinHTTPClient();
			virtual ~WinHTTPClient();

			bool Init() override;

			void UseAuthentication(const std::string& username, const std::string& password) override;

			bool GetText(const std::string& url, const std::string& postData,
				std::function<void(int httpStatusCode, const std::string& responseBody)> callback) override;

			void AllowInsecureTLS(bool allow) override { m_allowInsecureTLS = allow; }
		private:
			std::wstring m_authUsername;
			std::wstring m_authPassword;

			bool m_allowInsecureTLS;

			class Request;
			typedef std::shared_ptr<Request> PRequest;

			HINTERNET m_hSession;
			std::map<std::wstring, std::map<unsigned short, HINTERNET>> m_hHostConnects; // host + port -> HINTERNET from WinHttpConnect
			std::map<HINTERNET, PRequest> m_activeRequests;
			std::mutex m_access;

			static void CALLBACK WinHttpCallback(HINTERNET, DWORD_PTR, DWORD, LPVOID, DWORD);

			bool FindRequest(HINTERNET hRequest, PRequest& request);

			typedef enum {
				HTTP_GET  = 1,
				HTTP_POST = 2,
				// others not supported.
			} HTTPVerb;

			typedef enum {
				TLS_OFF,
				TLS_ON,
				TLS_ALLOW_INSECURE,
			} TLSFlag;

			class Request
			{
			public:
				Request(HINTERNET hSession, HINTERNET hConnect = 0);
				~Request();

				// void SetServer(const std::wstring& host, unsigned short port) {  } // not used
				void SetPath(const std::wstring& p) { m_path = p; }
				void SetTLS(TLSFlag f) { m_tls = f; }
				void SetVerb(HTTPVerb v) { m_verb = v; }
				void SetAuth(const std::wstring& username, const std::wstring& password) { m_username = username; m_password = password; }
				void SetPostData(const std::string& postData) { m_postData = postData; }

				void SetCompletionHandler(const std::function<void()> callback) { m_completionHandler = callback; }

				bool SendRequest();

				HINTERNET GetHandle() const { return m_hRequest; }

				void OnRequestSent();
				void OnHeadersComplete();
				void OnReadData(DWORD data_length);
				void OnComplete();
				void OnNetworkError(DWORD code, const std::wstring& description);

				int GetStatusCode() const;
				const std::wstring GetStatusText() const;
				std::string&& ExtractReadBuffer() { return std::move(m_readBuffer); }
				bool IsCanceled() const { return m_canceled; }
				int GetLastErrorCode() const { return m_lastErrorCode; }
				const std::wstring GetLastErrorDescription() const { return m_lastErrorDescription; }

				void Cancel();

			private:
				HINTERNET m_hConnect;
				HINTERNET m_hRequest;
				bool m_ownsHConnect;

				TLSFlag m_tls;
				HTTPVerb m_verb;
				std::wstring m_path;
				std::wstring m_username;
				std::wstring m_password;
				std::string m_postData;

				int m_httpStatusCode;
				std::string m_readBuffer;

				bool m_canceled;
				int m_lastErrorCode;
				std::wstring m_lastErrorDescription;

				std::function<void()> m_completionHandler;

				void InvokeCompletionHandler();
				void InternalError();
			};

			bool CreateRequest(const std::string& url, PRequest& request);
		};
	}
}

#endif /* !_WIN_HTTP_CLIENT_H */
