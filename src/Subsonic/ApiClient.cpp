/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#include "stdafx.h"
#include "Subsonic/ApiClient.h"
#include "subsonic-rest-api-pskel.hxx"

using namespace Player95::Subsonic;
namespace SubsonicXml = restapi;

// I would have preferred to use JSON for the API, but the JSON response
// is really flawed because it's nothing but a 1:1 translation from XML
// and even includes &#nnn; entities... so I guess XML is fine too.

ApiClient::ApiClient(IHTTPClient* httpClient, const ServerLoginCredentials& serverLogin) :
	m_httpClient(httpClient), m_serverLogin(serverLogin),
	m_restApiVersion("1.10.2"), m_restApiClientName("Player95")
{
	m_httpClient->UseAuthentication(m_serverLogin.GetUsername(), m_serverLogin.GetPassword());
}

std::string ApiClient::ControllerUrl(const std::string& action)
{
	// :TODO: make more efficient
	return m_serverLogin.GetHttpBaseUrl() + action + ".view?v=" + m_restApiVersion + "&c=" + m_restApiClientName;
}

bool ApiClient::PingServer(const std::function<void(bool networkStatus, bool responseStatus, const std::string& debugHint)> callback)
{
	return m_httpClient->GetText(ControllerUrl("ping"), "", [] (int httpStatusCode, const std::string& responseBody)
	{
		if (httpStatusCode == 200)
		{
			SubsonicXml::Response_pskel response;

			xml_schema::document doc_p(response, "http://subsonic.org/restapi", "subsonic-response");

			response.pre();
			doc_p.parse(responseBody.c_str(), responseBody.size(), true);
			response.post_Response();
		}
		else
		{
			// :TODO:
		}
	});
}

/*

class MusicFolderReader : public restapi::MusicFolder_pskel
{
virtual void
id(int id)
{
std::cout << "id = " << id << std::endl;
}

virtual void
name(const ::std::string& name)
{
std::cout << "name = " << name << std::endl;
}
};


restapi::Response_pskel response;
restapi::MusicFolders_pskel folders;
MusicFolderReader fr;

xml_schema::string_pimpl string_p;
xml_schema::int_pimpl int_p;

fr.name_parser(string_p);
fr.id_parser(int_p);

folders.musicFolder_parser(fr);
response.musicFolders_parser(folders);

xml_schema::document doc_p(response, "http://subsonic.org/restapi", "subsonic-response");

response.pre();
doc_p.parse("C:\\temp\\getMusicFolders.view.xml");
response.post_Response();

return 0;

*/
