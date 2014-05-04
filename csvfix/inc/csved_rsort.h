//---------------------------------------------------------------------------
// csved_rsort.h
//
// Do in-row sortingof CSV fields
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_RSORT_H
#define INC_CSVED_RSORT_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class RowSortCommand : public Command {

	public:

		RowSortCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );

};

//------------------------------------------------------------------------

}	// end namespace

#endif

