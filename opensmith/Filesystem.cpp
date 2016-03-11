#include "Filesystem.h"
#include "Settings/Settings.h"
#ifdef _WIN32
#include <dirent-win.h>
#else
#include <dirent.h>
#endif

void getFiles(std::vector<std::string>& fileList)
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(o.psarcDirectory.c_str())) == NULL)
		return;
	
	while ((ent = readdir(dir)) != NULL)
		if (ent->d_type == DT_REG)
			fileList.push_back(ent->d_name);
	closedir(dir);
	
	return;
}
