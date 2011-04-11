/*
 * CMessages.h
 *
 *  Created on: 16.07.2009
 *      Author: gerstrong
 */

#ifndef CMESSAGES_H_
#define CMESSAGES_H_

#include "../keen.h"
#include <string>
#include <map>

class CMessages {
public:	
	CMessages(unsigned char *p_exebuf, char episode, int version);
	
	bool extractGlobalStrings();
	
private:
	std::pair<std::string, std::string>
	extractString( std::string matchingstring, unsigned long start, unsigned long end, long offset=0 );

	bool extractEp4Strings();

	unsigned char *mp_exe;
	char m_episode;
	int m_version;
};

#endif /* CMESSAGES_H_ */
