#ifndef STEREOTYPE_H_INCLUDED
#define STEREOTYPE_H_INCLUDED

#include <Arduino.h>


class Stereotype {
protected:
	Stereotype (const char* const _tag): tag (_tag) {
	}

public:
	const char* const tag;

  virtual bool operator== (Stereotype const& other) = 0;

  virtual void clear () = 0;

	virtual boolean unmarshal (char *s) = 0;

	virtual char* marshal (char* buf, unsigned int size) = 0;
};

#endif
