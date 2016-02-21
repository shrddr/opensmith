#pragma once
#include "bitstream.h"

namespace {
	int ilog(unsigned int v)
	{
		int ret = 0;
		while (v) {
			ret++;
			v >>= 1;
		}
		return(ret);
	}
}


class CodebookLibrary
{
public:
	CodebookLibrary(const char* fileName);
	~CodebookLibrary();
	const char* getCodebook(int i) const
	{
		if (!codebookData || !codebookOffsets)
			throw std::string("codebook library not loaded");
		if (i >= codebookCount - 1 || i < 0)
			return NULL;
		return &codebookData[codebookOffsets[i]];
	}
	long getCodebookSize(int i) const
	{
		if (!codebookData || !codebookOffsets)
			throw std::string("codebook library not loaded");
		if (i >= codebookCount - 1 || i < 0)
			return -1;
		return codebookOffsets[i + 1] - codebookOffsets[i];
	}
	void rebuild(int i, OggStream& bos);
	void rebuild(BitStream &bis, unsigned long cb_size, OggStream& bos);
private:
	char* codebookData;
	long* codebookOffsets;
	long codebookCount;
};

