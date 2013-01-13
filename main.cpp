#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <list>

using namespace std;

static void str_html_escape(string &p_str)
{
	size_t pos = 0;

	// & -> &amp;
	pos = p_str.find_first_of('&');
	while(pos != p_str.npos)
	{
		p_str.replace(pos, 1, "&amp;");
		pos = p_str.find_first_of('&', pos + 1);
	}

	// < -> &lt;
	pos = p_str.find_first_of('<');
	while(pos != p_str.npos)
	{
		p_str.replace(pos, 1, "&lt;");
		pos = p_str.find_first_of('<', pos + 1);
	}

	// > -> &gt;
	pos = p_str.find_first_of('>');
	while(pos != p_str.npos)
	{
		p_str.replace(pos, 1, "&gt;");
		pos = p_str.find_first_of('>', pos + 1);
	}

	// " -> &quot;
	pos = p_str.find_first_of('"');
	while(pos != p_str.npos)
	{
		p_str.replace(pos, 1, "&quot;");
		pos = p_str.find_first_of('"', pos + 1);
	}

	// \n -> <br />
	pos = p_str.find_first_of('\n');
	while(pos != p_str.npos)
	{
		p_str.replace(pos, 1, "<br />");
		pos = p_str.find_first_of('\n', pos + 1);
	}
}

struct GameDetectionSet
{
	string m_name;
	string m_shortName;
	string m_detectServer;
	string m_serverBroadCastPort;
	string m_serverGameName;
	string m_serverStatusType;
	string m_excludeIPPort;
	string m_launchPasswordArgs;
	string m_launchNetworkArgs;
	string m_launchArgs;

	bool m_voice;

	// For webgames
	bool m_external;
	list<string> m_urls;
	string m_launcherUrl;

	list<string> m_req_args;
	list<string> m_bad_args;

	GameDetectionSet() { m_external = m_voice = false; m_detectServer = "enabled"; }

	void parseDetectionSet(ifstream &p_file)
	{
		string line;
		streampos old_pos;
		while(!p_file.eof())
		{
			// Store old position to reset in case of a new game
			old_pos = p_file.tellg();

			getline(p_file, line);

			if(line.empty())
				continue;

			// New game starts? Reset to old position and break the loop
			if(line[0] == '[')
			{
				p_file.seekg(old_pos);
				break;
			}

			size_t eq_pos = line.find_first_of('=');

			// Invalid line?
			if(eq_pos == line.npos)
				continue;

			string entry = line.substr(0, eq_pos);
			string value = line.substr(eq_pos + 1, line.find_first_of("\r\n") - eq_pos - 1);

			// Skip comments
			if(entry[0] == ';')
				continue;

			str_html_escape(value);

			if(entry.compare("DetectServer") == 0)
			{
				m_detectServer = value;

				if(m_detectServer.empty())
					m_detectServer = "enabled";
				else if(m_detectServer.compare("FALSE") == 0)
					m_detectServer = "disabled";
				else if(m_detectServer.compare("TRUE") == 0)
					m_detectServer = "enabled";
			}
			else if(entry.compare("ServerBroadcastPort") == 0)
			{
				while(value.find(',') != value.npos)
					value.replace(value.find(','), 1, 1, ';');

				m_serverBroadCastPort = value;
			}
			else if(entry.compare("ServerGameName") == 0)
				m_serverGameName = value;
			else if(entry.compare("ServerStatusType") == 0)
				m_serverStatusType = value;
			else if(entry.compare("ExcludeIPPorts") == 0)
			{
				while(value.find(',') != value.npos)
					value.replace(value.find(','), 1, 1, ';');

				m_excludeIPPort = value;
			}
			else if(entry.compare("LauncherPasswordArgs") == 0)
				m_launchPasswordArgs = value;
			else if(entry.compare("LauncherNetworkArgs") == 0)
				m_launchNetworkArgs = value;
			else if(entry.compare("Launch") == 0)
				m_launchArgs = value;
			else if(entry.compare("LongName") == 0)
				m_name = value;
			else if(entry.compare("ShortName") == 0)
				m_shortName = value;
			else if(entry.compare("External") == 0)
				m_external = true;
			else if(entry.compare(0, 7, "GameUrl") == 0)
				m_urls.push_back(value);
			else if(entry.compare("LauncherUrl") == 0)
				m_launcherUrl = value;
			else if(entry.find("CommandLineMustContain") != entry.npos)
				m_req_args.push_back(value);
			else if(entry.find("CommandLineMustNotContain") != entry.npos)
				m_bad_args.push_back(value);
			else if(entry.compare("SoftwareType") == 0)
			{
				if(value.compare("VoiceChat") == 0)
					m_voice = true;
			}
		}
	}

	void writeToFile(ofstream &p_file)
	{
		p_file << "		<detection>" << endl;
		p_file << "			<server_detection>" << m_detectServer << "</server_detection>" << endl;

		if(!m_excludeIPPort.empty())
			p_file << "			<server_excluded_ports>" << m_excludeIPPort << "</server_excluded_ports>" << endl;
		if(!m_serverBroadCastPort.empty())
			p_file << "			<server_broadcast_ports>" << m_serverBroadCastPort << "</server_broadcast_ports>" << endl;
		if(!m_serverGameName.empty())
			p_file << "			<server_game_name>" << m_serverGameName << "</server_game_name>" << endl;
		if(!m_serverStatusType.empty())
			p_file << "			<server_status_type>" << m_serverStatusType << "</server_status_type>" << endl;
		if(!m_launchPasswordArgs.empty())
			p_file << "			<launch_password_args>" << m_launchPasswordArgs << "</launch_password_args>" << endl;
		if(!m_launchNetworkArgs.empty())
			p_file << "			<launch_network_args>" << m_launchNetworkArgs << "</launch_network_args>" << endl;
		if(!m_launchArgs.empty())
			p_file << "			<launch_args>" << m_launchArgs << "</launch_args>" << endl;
		if(!m_req_args.empty() || !m_bad_args.empty())
		{
			p_file << "			<arguments";
			if(!m_req_args.empty())
			{
				p_file << " required=\"";
				list<string>::iterator iter;
				for(iter = m_req_args.begin(); iter != m_req_args.end(); iter++)
				{
					if(iter != m_req_args.begin())
						p_file << ";";

					p_file << *iter;
				}
				p_file << "\"";
			}
			if(!m_bad_args.empty())
			{
				p_file << " invalid=\"";
				list<string>::iterator iter;
				for(iter = m_bad_args.begin(); iter != m_bad_args.end(); iter++)
				{
					if(iter != m_bad_args.begin())
						p_file << ";";

					p_file << *iter;
				}
				p_file << "\"";
			}
			p_file << " />" << endl;
		}
		if(m_external)
		{
			p_file << "			<external ";
			if(!m_urls.empty())
			{
				p_file << "url=\"";
				list<string>::iterator iter;
				for(iter = m_urls.begin(); iter != m_urls.end(); iter++)
				{
					if(iter != m_urls.begin())
						p_file << ';';
					p_file << *iter;
				}
				p_file << "\" ";
			}
			if(!m_launcherUrl.empty())
				p_file << "launchurl=\"" << m_launcherUrl << "\" ";
			p_file << "/>" << endl;
		}

		p_file << "		</detection>" << endl;
	}
};

struct XMLGameNode
{
	int m_id;
	string m_name;
	string m_shortName;

	bool m_voice;

	list<GameDetectionSet> m_detectSets;

	XMLGameNode() { m_id = -2; }

	void parseGame(ifstream &p_file)
	{
		string line;
		streampos old_pos;
		while(!p_file.eof())
		{
			// Store old position to reset in case of a new game
			old_pos = p_file.tellg();

			getline(p_file, line);

			if(line.empty())
				continue;

			if(line[0] == '[')
			{
				// Extract ID from the line
				string id_str_with_subid = line.substr(1, line.find_last_of(']') - 1);
				string id_str = id_str_with_subid.substr(0, id_str_with_subid.find_first_of('_'));

				stringstream convert(id_str);
				int id;
				convert >> id;

				// Get the detection set if we haven't any ID yet or the ID is the same as ours
				if(m_id == -2 || m_id == id)
				{
					m_id = id;

					GameDetectionSet newSet;
					newSet.parseDetectionSet(p_file);

					// Disable server detection for game -1
					if(m_id == -1)
						newSet.m_detectServer = "disabled";

					m_detectSets.push_back(newSet);
				}
				// Restore the old position and quit the loop
				else
				{
					p_file.seekg(old_pos);
					break;
				}
			}
			else
				break;
		}

		// Get the game name from the first set
		if(!m_detectSets.empty())
		{
			m_name = m_detectSets.front().m_name;
			m_shortName = m_detectSets.front().m_shortName;
			m_voice = m_detectSets.front().m_voice;
		}
	}

	void writeToFile(ofstream &p_file)
	{
		p_file << "	<game id=\"" << m_id << "\" name=\"" << m_name << "\" shortname=\"" << m_shortName << "\">" << endl;

		// Write detection sets
		list<GameDetectionSet>::iterator iter;
		for(iter = m_detectSets.begin(); iter != m_detectSets.end(); iter++)
			iter->writeToFile(p_file);

		if(m_voice)
			p_file << "		<voice/>" << endl;

		p_file << "	</game>" << endl;
	}
};

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		cout << "Usage: " << argv[0] << " xfire_games.ini gfire_games.xml" << endl;
		return 0;
	}

	ifstream xfire_ini(argv[1]);
	ofstream gfire_xml(argv[2]);

	if(xfire_ini.fail())
	{
		cout << "Unable to open file \"" << argv[1] << "\" for reading!" << endl;
		return 0;
	}
	else if(gfire_xml.fail())
	{
		cout << "Unable to open file \"" << argv[2] << "\" for writing!" << endl;
		return 0;
	}

	// Write XML head and main node
	gfire_xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;

	// Write Game List Version
	time_t rawtime;
	time(&rawtime);

	gfire_xml << "<games version=\"" << rawtime << "\">" << endl;
	tm *utctime = gmtime(&rawtime);

	char time_str[50];
	strftime(time_str, 50, "%d %B %Y", utctime);

	gfire_xml << "	<game id=\"100\" name=\"" << time_str << "\" />" << endl;

	int count = 0;

	// Remove the version lines
	string line;
	getline(xfire_ini, line);
	getline(xfire_ini, line);
	getline(xfire_ini, line);

	// Loop through the file
	while(!xfire_ini.eof())
	{
		XMLGameNode node;
		node.parseGame(xfire_ini);
		node.writeToFile(gfire_xml);

		count++;
	}

	// Write end of main node
	gfire_xml << "</games>";

	xfire_ini.close();
	gfire_xml.close();

	cout << "Processed " << count << " game entries" << endl << "Finished" << endl;
}
