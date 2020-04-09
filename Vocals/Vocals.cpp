#include <iostream>
#include "Vocals.h"
#include "../tinyxml2/tinyxml2.h"


Vocals::Vocals(std::vector<char> vocalsEntryStorage)
{
	tinyxml2::XMLDocument doc;
	doc.Parse((const char*)vocalsEntryStorage.data(), vocalsEntryStorage.size());
	std::cout << doc.Error() << std::endl;
	tinyxml2::XMLElement* vocals = doc.FirstChildElement("vocals");
	
	VocalsEntry entry;
	for (tinyxml2::XMLElement* e = vocals->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
	{
		entry.time = e->FloatAttribute("time");
		entry.tlen = e->FloatAttribute("length");
		entry.text = e->Attribute("lyric");
		records.push_back(entry);
		std::cout << entry.time << " / " << entry.tlen << " / " << entry.text << std::endl;
	}
}

size_t Vocals::Count()
{
	return records.size();
}