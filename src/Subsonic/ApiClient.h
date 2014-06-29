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

			// these methods return true if the input etc. is fine and the callback will be invoked.
			// if false is returned, the callback will NOT be invoked.

			// users must not make any assumptions about the thread that the callback is executed in!

			bool PingServer(const std::function<void (bool networkStatus, bool responseStatus, const std::string& debugHint)> callback);

		protected:
			IHTTPClient* m_httpClient;
			ServerLoginCredentials m_serverLogin;

			std::string m_restApiVersion;
			std::string m_restApiClientName;

			std::string ControllerUrl(const std::string& action);
		};
	}
}

#endif /* !_API_CLIENT_H */
