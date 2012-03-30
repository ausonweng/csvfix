//---------------------------------------------------------------------------
// csved_eval.cpp
//
// eval command for csvfix - does expression evaluation
// this doesn't do much - the heavy lifting is in alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_collect.h"
#include "csved_cli.h"
#include "csved_eval.h"
#include "csved_strings.h"
#include "csved_evalvars.h"

using std::string;

namespace CSVED {

//----------------------------------------------------------------------------
// Register eval command
//----------------------------------------------------------------------------

static RegisterCommand <EvalCommand> rc1_(
	CMD_EVAL,
	"expression evaluation"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const EVAL_HELP = {
	"performs expression evaluation on fields, adding results as new fields\n"
	"usage: csvfix eval [flags] [file ...]\n"
	"where flags are:\n"
	"  -e expr\texpression to evaluate\n"
	"  -r f,expr\treplace field f with result of evaluating expr\n"
	"#ALL"
};

//----------------------------------------------------------------------------
// Standard command ctor
//----------------------------------------------------------------------------

EvalCommand ::	EvalCommand( const string & name,
									 const string & desc )
		: Command( name, desc, EVAL_HELP ) {
		AddFlag( ALib::CommandLineFlag( FLAG_EXPR, false, 1, true ) );
		AddFlag( ALib::CommandLineFlag( FLAG_REMOVE, false, 1, true ) );
}

//----------------------------------------------------------------------------
// Compile all expressions and then evaluate for each input row
//----------------------------------------------------------------------------

int EvalCommand ::	Execute( ALib::CommandLine & cmd ) {

	IOManager io( cmd );
	CSVRow row;

	GetExpressions( cmd );

	while( io.ReadCSV( row ) ) {
		SetParams( row, io );
		Evaluate( row );
		io.WriteRow( row );
	}

	return 0;
}

//----------------------------------------------------------------------------
// Evaluate expressions. If the field index associated with the expression
// is negative, append to row, otherwise replace field specified by the
// index if possible.
//----------------------------------------------------------------------------

void EvalCommand ::	Evaluate( CSVRow & row ) {
	for ( unsigned int i = 0; i < mFieldExprs.size() ; i++ ) {
		string r = mFieldExprs[i].mExpr.Evaluate();
		if ( mFieldExprs[i].mField < 0 || mFieldExprs[i].mField >= row.size() ) {
			row.push_back( r );
		}
		else {
			row[ mFieldExprs[i].mField ] = r;
		}
	}
}

//----------------------------------------------------------------------------
// Set positional parameters (each such parameter is a field in the CSV input)
// and special named constants
//----------------------------------------------------------------------------

void EvalCommand ::	SetParams( const CSVRow & row, IOManager & iom ) {
	for ( unsigned int i = 0; i < mFieldExprs.size(); i++ ) {
		ALib::Expression & e = mFieldExprs[i].mExpr;
		AddVars( e, iom, row );
	}
}

//----------------------------------------------------------------------------
// Options may be of the form:
//
//     -e expr
//     -r field,expr
//
// In the first case, create a dummy field index with negative value.
//----------------------------------------------------------------------------

void EvalCommand ::	GetExpressions( ALib::CommandLine & cmd ) {
	int i = 2;	// skip exe name and command name

	while( i < cmd.Argc() ) {
		if ( cmd.Argv( i ) == FLAG_EXPR ) {
			if ( i + 1 >= cmd.Argc() ) {
				CSVTHROW( "Missing expression" );
			}
			i++;
			string expr = cmd.Argv( i );
			ALib::Expression ex;
			string emsg = ex.Compile( expr );
			if ( emsg != "" ) {
				CSVTHROW( emsg + " in " + expr );
			}
			mFieldExprs.push_back( FieldEx( -1, ex ) );
		}
		else if ( cmd.Argv( i ) == FLAG_REMOVE ) {
			if ( i + 1 >= cmd.Argc() ) {
				CSVTHROW( "Missimg field/expression" );
			}
			i++;
			string::size_type pos = cmd.Argv(i).find_first_of( "," );
			if ( pos == string::npos ) {
				CSVTHROW( "Invalid field/index pair: " << cmd.Argv(i) );
			}
			string field = cmd.Argv(i).substr( 0, pos );
			string expr = cmd.Argv(i).substr( pos + 1 );

			if ( ! ALib::IsInteger( field ) ) {
				CSVTHROW( "Invalid field (need integer): " << field );
			}
			int n = ALib::ToInteger( field );
			if ( n <= 0 ) {
				CSVTHROW( "Invalid field (must be greater than zero): " << field );
			}
			ALib::Expression ex;
			string emsg = ex.Compile( expr );
			if ( emsg != "" ) {
				CSVTHROW( emsg + " in " + expr );
			}

			mFieldExprs.push_back( FieldEx( n - 1, ex ) );
		}
		i++;
	}
	if ( mFieldExprs.size() == 0 ) {
		CSVTHROW( "Need at least one of -e or -r options" );
	}
}

//------------------------------------------------------------------------

} // namespace

// end

