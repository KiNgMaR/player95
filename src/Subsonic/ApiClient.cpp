/**
* player95 - (c) Ingmar Runge 2014
*
* License: MIT
**/

#include "stdafx.h"
#include "Subsonic/ApiClient.h"
#include "subsonic-rest-api-pskel.hxx"

using namespace Player95::Subsonic;
namespace SubsonicXmlApi = restapi;

ApiClient::ApiClient(IHTTPClient* httpClient, const ServerLoginCredentials& serverLogin)
	: m_httpClient(httpClient), m_serverLogin(serverLogin)
{
	
}

/*

#include "subsonic-rest-api-pskel.hxx"

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
