/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#ifndef _API_CLIENT_H
#define _API_CLIENT_H

#include <string>
#include "Subsonic/ServerLoginCredentials.h"
#include "Common/IHTTPClient.h"

namespace Player95
{
	namespace Subsonic
	{
		using Common::IHTTPClient;

		class ApiClient
		{
		public:
			ApiClient(IHTTPClient* httpClient, const ServerLoginCredentials& serverLogin);

			typedef enum {
				SAE__NO_ERROR_SET     = -1,
				SAE_GENERIC           = 00,
				SAE_MISSING_PARAMETER = 10,
				SAE_OUTDATED_CLIENT   = 20,
				SAE_OUTDATED_SERVER   = 30,
				SAE_BAD_USER_PASSWORD = 40,
				SAE_ACCESS_DENIED     = 50,
				SAE_LICENSE_ISSUE     = 60,
				SAE_NOT_FOUND         = 70,
			} APIErrorCode;

			bool pingServer(const std::function<void (bool network_status, bool response_status, APIErrorCode error_code, const std::string& debug_hint)> callback);

		protected:
			IHTTPClient* m_httpClient;
			ServerLoginCredentials m_serverLogin;
		};
	}
}

#endif /* !_API_CLIENT_H */
