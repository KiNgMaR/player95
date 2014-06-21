/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#ifndef _SERVER_LOGIN_CREDENTIALS_H
#define _SERVER_LOGIN_CREDENTIALS_H

#include <string>
#include <sstream>

namespace Player95
{
	namespace Subsonic
	{
		class ServerLoginCredentials
		{
		public:
			ServerLoginCredentials(const std::string& hostname, unsigned short port, bool ssl)
				: m_hostname(hostname), m_port(port), m_ssl(ssl)
			{
			}

			void SetLogin(const std::string& username, const std::string& password)
			{
				m_username = username;
				m_password = password;
			}

			const std::string GetHttpBaseUrl() const
			{
				std::stringstream s;

				s << (m_ssl ? "https://" : "http://");
				s << m_hostname;
				if (m_port != (m_ssl ? 443 : 80))
					s << ":" << m_port;
				s << "/rest/";

				return s.str();
			}

			const std::string& GetHostname() const { return m_hostname; }
			unsigned short GetPort() const { return m_port; }
			bool IsSSL() const { return m_ssl; }
			const std::string& GetUsername() const { return m_username; }
			const std::string& GetPassword() const { return m_password; }

		protected:
			std::string m_hostname;
			unsigned short m_port;
			bool m_ssl;
			std::string m_username;
			std::string m_password;
		};
	}
}

#endif /* !_SERVER_LOGIN_CREDENTIALS_H */
