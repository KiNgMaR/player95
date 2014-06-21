/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#ifndef _IHTTP_CLIENT_H
#define _IHTTP_CLIENT_H

#include <string>
#include <functional>

namespace Player95
{
	namespace Common
	{
		// this is not very universal, it only caters to what Player95 needs.

		class IHTTPClient
		{
		public:
			virtual void UseAuthentication(const std::string& username, const std::string& password) = 0;

			virtual bool GetText(const std::string& url,
				std::function<void (int http_status_code, const std::string& response_body)> callback) = 0;

			virtual ~IHTTPClient() {}
		};
	}
}

#endif /* !_IHTTP_CLIENT_H */
