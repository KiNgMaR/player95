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

			// usually HTTP libraries need some kind of initialization, this can be performed here:
			virtual bool Init() = 0;

			virtual bool GetText(const std::string& url, const std::string& postData,
				std::function<void (int httpStatusCode, const std::string& responseBody)> callback) = 0;

			virtual void AllowInsecureTLS(bool allow) = 0;

			virtual ~IHTTPClient() {}
		};
	}
}

#endif /* !_IHTTP_CLIENT_H */
