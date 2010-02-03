//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef KEYVALUES_H
#define KEYVALUES_H

#ifdef _WIN32
#pragma once
#endif

// #include <vgui/VGUI.h>

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#include "utlvector.h"
#include "Color.h"

class IBaseFileSystem;
class CUtlBuffer;
class Color;
typedef void * FileHandle_t;

//-----------------------------------------------------------------------------
// Purpose: Simple recursive data access class
//			Used in vgui for message parameters and resource files
//			Destructor deletes all child KeyValues nodes
//			Data is stored in key (string names) - (string/int/float)value pairs called nodes.
//
//	About KeyValues Text File Format:

//	It has 3 control characters '{', '}' and '"'. Names and values may be quoted or
//	not. The quote '"' charater must not be used within name or values, only for
//	quoting whole tokens. You may use escape sequences wile parsing and add within a
//	quoted token a \" to add quotes within your name or token. When using Escape
//	Sequence the parser must now that by setting KeyValues::UsesEscapeSequences( true ),
//	which it's off by default. Non-quoted tokens ends with a whitespace, '{', '}' and '"'.
//	So you may use '{' and '}' within quoted tokens, but not for non-quoted tokens.
//  An open bracket '{' after a key name indicates a list of subkeys which is finished
//  with a closing bracket '}'. Subkeys use the same definitions recursively.
//  Whitespaces are space, return, newline and tabulator. Allowed Escape sequences
//	are \n, \t, \\, \n and \". The number character '#' is used for macro purposes 
//	(eg #include), don't use it as first charater in key names.
//-----------------------------------------------------------------------------
class KeyValues
{
public:
	KeyValues( const char *setName );

#if SOURCE_ENGINE >= SE_ORANGEBOX
	//
	// AutoDelete class to automatically free the keyvalues.
	// Simply construct it with the keyvalues you allocated and it will free them when falls out of scope.
	// When you decide that keyvalues shouldn't be deleted call Assign(NULL) on it.
	// If you constructed AutoDelete(NULL) you can later assign the keyvalues to be deleted with Assign(pKeyValues).
	// You can also pass temporary KeyValues object as an argument to a function by wrapping it into KeyValues::AutoDelete
	// instance:   call_my_function( KeyValues::AutoDelete( new KeyValues( "test" ) ) )
	//
	class AutoDelete
	{
	public:
		explicit inline AutoDelete( KeyValues *pKeyValues ) : m_pKeyValues( pKeyValues ) {}
		inline ~AutoDelete( void ) { if( m_pKeyValues ) m_pKeyValues->deleteThis(); }
		inline void Assign( KeyValues *pKeyValues ) { m_pKeyValues = pKeyValues; }
	private:
		AutoDelete( AutoDelete const &x ); // forbid
		AutoDelete & operator= ( AutoDelete const &x ); // forbid
		KeyValues *m_pKeyValues;
	};
#endif//ORANGEBOX

	// Quick setup constructors
	KeyValues( const char *setName, const char *firstKey, const char *firstValue );
	KeyValues( const char *setName, const char *firstKey, const wchar_t *firstValue );
	KeyValues( const char *setName, const char *firstKey, int firstValue );
	KeyValues( const char *setName, const char *firstKey, const char *firstValue, const char *secondKey, const char *secondValue );
	KeyValues( const char *setName, const char *firstKey, int firstValue, const char *secondKey, int secondValue );

	// Section name
	const char *GetName() const;
	void SetName( const char *setName);

	// gets the name as a unique int
	int GetNameSymbol() const;

	// File access. Set UsesEscapeSequences true, if resource file/buffer uses Escape Sequences (eg \n, \t)
	char IsUsingEscapeSequences(); // Returns whether the KeyValues is using escape sequences.
	void UsesEscapeSequences(bool state); // default false
	bool LoadFromFile( IBaseFileSystem *filesystem, const char *resourceName, const char *pathID = NULL );
	bool SaveToFile( IBaseFileSystem *filesystem, const char *resourceName, const char *pathID = NULL);

	// Read from a buffer...  Note that the buffer must be null terminated
	bool LoadFromBuffer( char const *resourceName, const char *pBuffer, IBaseFileSystem* pFileSystem = NULL, const char *pPathID = NULL );

	// Read from a utlbuffer...
	bool LoadFromBuffer( char const *resourceName, CUtlBuffer &buf, IBaseFileSystem* pFileSystem = NULL, const char *pPathID = NULL );

	// Find a keyValue, create it if it is not found.
	// Set bCreate to true to create the key if it doesn't already exist (which ensures a valid pointer will be returned)
	KeyValues *FindKey(const char *keyName, bool bCreate = false);
	KeyValues *FindKey(int keySymbol) const;
	KeyValues *CreateNewKey();		// creates a new key, with an autogenerated name.  name is guaranteed to be an integer, of value 1 higher than the highest other integer key name
	void AddSubKey( KeyValues *pSubkey );	// Adds a subkey. Make sure the subkey isn't a child of some other keyvalues
	void RemoveSubKey(KeyValues *subKey);	// removes a subkey from the list, DOES NOT DELETE IT

	// Key iteration.
	//
	// NOTE: GetFirstSubKey/GetNextKey will iterate keys AND values. Use the functions 
	// below if you want to iterate over just the keys or just the values.
	//
	KeyValues *GetFirstSubKey();	// returns the first subkey in the list
	KeyValues *GetNextKey();		// returns the next subkey
	void SetNextKey( KeyValues * pDat);

	//
	// These functions can be used to treat it like a true key/values tree instead of 
	// confusing values with keys.
	//
	// So if you wanted to iterate all subkeys, then all values, it would look like this:
	//     for ( KeyValues *pKey = pRoot->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	//     {
	//		   Msg( "Key name: %s\n", pKey->GetName() );
	//     }
	//     for ( KeyValues *pValue = pRoot->GetFirstValue(); pKey; pKey = pKey->GetNextValue() )
	//     {
	//         Msg( "Int value: %d\n", pValue->GetInt() );  // Assuming pValue->GetDataType() == TYPE_INT...
	//     }
	KeyValues* GetFirstTrueSubKey();
	KeyValues* GetNextTrueSubKey();

	KeyValues* GetFirstValue();	// When you get a value back, you can use GetX and pass in NULL to get the value.
	KeyValues* GetNextValue();


	// Data access
	int   GetInt( const char *keyName = NULL, int defaultValue = 0 );
	uint64 GetUint64( const char *keyName = NULL, uint64 defaultValue = 0 );
	float GetFloat( const char *keyName = NULL, float defaultValue = 0.0f );
	const char *GetString( const char *keyName = NULL, const char *defaultValue = "" );
	const wchar_t *GetWString( const char *keyName = NULL, const wchar_t *defaultValue = L"" );
	void *GetPtr( const char *keyName = NULL, void *defaultValue = (void*)0 );
	Color GetColor( const char *keyName = NULL /* default value is all black */);
	bool  IsEmpty(const char *keyName = NULL);

	// Data access
	int   GetInt( int keySymbol, int defaultValue = 0 );
	float GetFloat( int keySymbol, float defaultValue = 0.0f );
	const char *GetString( int keySymbol, const char *defaultValue = "" );
	const wchar_t *GetWString( int keySymbol, const wchar_t *defaultValue = L"" );
	void *GetPtr( int keySymbol, void *defaultValue = (void*)0 );
	Color GetColor( int keySymbol /* default value is all black */);
	bool  IsEmpty( int keySymbol );

	// Key writing
	void SetWString( const char *keyName, const wchar_t *value );
	void SetString( const char *keyName, const char *value );
	void SetInt( const char *keyName, int value );
	void SetUint64( const char *keyName, uint64 value );
	void SetFloat( const char *keyName, float value );
	void SetPtr( const char *keyName, void *value );
	void SetColor( const char *keyName, Color value);

	// Memory allocation (optimized)
	void *operator new( size_t iAllocSize );
	void *operator new( size_t iAllocSize, int nBlockUse, const char *pFileName, int nLine );
	void operator delete( void *pMem );
	void operator delete( void *pMem, int nBlockUse, const char *pFileName, int nLine );

	KeyValues& operator=( KeyValues& src );

	// Adds a chain... if we don't find stuff in this keyvalue, we'll look
	// in the one we're chained to.
	void ChainKeyValue( KeyValues* pChain );
	
	void RecursiveSaveToFile( CUtlBuffer& buf, int indentLevel );

	bool WriteAsBinary( CUtlBuffer &buffer );
	bool ReadAsBinary( CUtlBuffer &buffer );

	// Allocate & create a new copy of the keys
	KeyValues *MakeCopy( void ) const;

	// Make a new copy of all subkeys, add them all to the passed-in keyvalues
	void CopySubkeys( KeyValues *pParent ) const;

	// Clear out all subkeys, and the current value
	void Clear( void );

	// Data type
	enum types_t
	{
		TYPE_NONE = 0,
		TYPE_STRING,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_PTR,
		TYPE_WSTRING,
		TYPE_COLOR,
		TYPE_UINT64,
		TYPE_NUMTYPES, 
	};
	types_t GetDataType(const char *keyName = NULL);

	// Virtual deletion function - ensures that KeyValues object is deleted from correct heap
	void deleteThis();

#if SOURCE_ENGINE >= SE_ORANGEBOX
	void SetStringValue( char const *strValue );

	// unpack a key values list into a structure
	void UnpackIntoStructure( struct KeyValuesUnpackStructure const *pUnpackTable, void *pDest );

	// Process conditional keys for widescreen support.
	bool ProcessResolutionKeys( const char *pResString );
#endif//ORANGEBOX
		
private:
	KeyValues( KeyValues& );	// prevent copy constructor being used

	// prevent delete being called except through deleteThis()
	~KeyValues();

	KeyValues* CreateKey( const char *keyName );
	
	void RecursiveCopyKeyValues( KeyValues& src );
	void RemoveEverything();
//	void RecursiveSaveToFile( IBaseFileSystem *filesystem, CUtlBuffer &buffer, int indentLevel );
//	void WriteConvertedString( CUtlBuffer &buffer, const char *pszString );
	
	// NOTE: If both filesystem and pBuf are non-null, it'll save to both of them.
	// If filesystem is null, it'll ignore f.
	void RecursiveSaveToFile( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, int indentLevel );
	void WriteConvertedString( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, const char *pszString );
	
	void RecursiveLoadFromBuffer( char const *resourceName, CUtlBuffer &buf );

	// For handling #include "filename"
	void AppendIncludedKeys( CUtlVector< KeyValues * >& includedKeys );
	void ParseIncludedKeys( char const *resourceName, const char *filetoinclude, 
		IBaseFileSystem* pFileSystem, const char *pPathID, CUtlVector< KeyValues * >& includedKeys );

#if SOURCE_ENGINE >= SE_ORANGEBOX
	// For handling #base "filename"
	void MergeBaseKeys( CUtlVector< KeyValues * >& baseKeys );
	void RecursiveMergeKeyValues( KeyValues *baseKV );
#endif//ORANGEBOX

	// NOTE: If both filesystem and pBuf are non-null, it'll save to both of them.
	// If filesystem is null, it'll ignore f.
	void InternalWrite( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, const void *pData, int len );
	
	void Init();
#if SOURCE_ENGINE >= SE_ORANGEBOX
	const char * ReadToken( CUtlBuffer &buf, bool &wasQuoted, bool &wasConditional );
#else//NOT ORANGEBOX
	const char * ReadToken( CUtlBuffer &buf, bool &wasQuoted );
#endif//ORANGEBOX
	void WriteIndents( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, int indentLevel );

	void FreeAllocatedValue();
	void AllocateValueBlock(int size);

	int m_iKeyName;	// keyname is a symbol defined in KeyValuesSystem

	// These are needed out of the union because the API returns string pointers
	char *m_sValue;
	wchar_t *m_wsValue;

	// we don't delete these
	union
	{
		int m_iValue;
		float m_flValue;
		void *m_pValue;
		unsigned char m_Color[4];
	};
	
#if SOURCE_ENGINE >= SE_ORANGEBOX
	char	   m_iDataType;
	char	   m_bHasEscapeSequences; // true, if while parsing this KeyValue, Escape Sequences are used (default false)
	char	   unused[2];

	KeyValues *m_pPeer;	// pointer to next key in list
	KeyValues *m_pSub;	// pointer to Start of a new sub key list
	KeyValues *m_pChain;// Search here if it's not in our list
#else//NOT ORANGEBOX
# ifdef _XBOX
	char	   m_iDataType;
	char	   m_bHasEscapeSequences; // true, if while parsing this KeyValue, Escape Sequences are used (default false)
	char	   reserved[2];

	KeyValues *m_pPeer;	// pointer to next key in list
	KeyValues *m_pSub;	// pointer to Start of a new sub key list
	KeyValues *m_pChain;// Search here if it's not in our list
# else
	char	   m_iDataType;
	char	   reserved[5];

	KeyValues *m_pPeer;	// pointer to next key in list
	KeyValues *m_pSub;	// pointer to Start of a new sub key list
	KeyValues *m_pChain;// Search here if it's not in our list
	char	   m_bHasEscapeSequences; // true, if while parsing this KeyValue, Escape Sequences are used (default false)
# endif
#endif//ORANGEBOX
};

#if SOURCE_ENGINE >= SE_ORANGEBOX
enum KeyValuesUnpackDestinationTypes_t
{
	UNPACK_TYPE_FLOAT,										// dest is a float
	UNPACK_TYPE_VECTOR,										// dest is a Vector
	UNPACK_TYPE_VECTOR_COLOR,								// dest is a vector, src is a color
	UNPACK_TYPE_STRING,										// dest is a char *. unpacker will allocate.
	UNPACK_TYPE_INT,										// dest is an int
	UNPACK_TYPE_FOUR_FLOATS,	 // dest is an array of 4 floats. source is a string like "1 2 3 4"
	UNPACK_TYPE_TWO_FLOATS,		 // dest is an array of 2 floats. source is a string like "1 2"
};

#define UNPACK_FIXED( kname, kdefault, dtype, ofs ) { kname, kdefault, dtype, ofs, 0 }
#define UNPACK_VARIABLE( kname, kdefault, dtype, ofs, sz ) { kname, kdefault, dtype, ofs, sz }
#define UNPACK_END_MARKER { NULL, NULL, UNPACK_TYPE_FLOAT, 0 }

struct KeyValuesUnpackStructure
{
	char const *m_pKeyName;									// null to terminate tbl
	char const *m_pKeyDefault;								// null ok
	KeyValuesUnpackDestinationTypes_t m_eDataType;			// UNPACK_TYPE_INT, ..
	size_t m_nFieldOffset;									// use offsetof to set
	size_t m_nFieldSize;									// for strings or other variable length
};
#endif//ORANGEBOX

//-----------------------------------------------------------------------------
// inline methods
//-----------------------------------------------------------------------------
inline int   KeyValues::GetInt( int keySymbol, int defaultValue )
{
	KeyValues *dat = FindKey( keySymbol );
	return dat ? dat->GetInt( (const char *)NULL, defaultValue ) : defaultValue;
}

inline float KeyValues::GetFloat( int keySymbol, float defaultValue )
{
	KeyValues *dat = FindKey( keySymbol );
	return dat ? dat->GetFloat( (const char *)NULL, defaultValue ) : defaultValue;
}

inline const char *KeyValues::GetString( int keySymbol, const char *defaultValue )
{
	KeyValues *dat = FindKey( keySymbol );
	return dat ? dat->GetString( (const char *)NULL, defaultValue ) : defaultValue;
}

inline const wchar_t *KeyValues::GetWString( int keySymbol, const wchar_t *defaultValue )
{
	KeyValues *dat = FindKey( keySymbol );
	return dat ? dat->GetWString( (const char *)NULL, defaultValue ) : defaultValue;
}

inline void *KeyValues::GetPtr( int keySymbol, void *defaultValue )
{
	KeyValues *dat = FindKey( keySymbol );
	return dat ? dat->GetPtr( (const char *)NULL, defaultValue ) : defaultValue;
}

inline Color KeyValues::GetColor( int keySymbol )
{
	Color defaultValue( 0, 0, 0, 0 );
	KeyValues *dat = FindKey( keySymbol );
	return dat ? dat->GetColor( ) : defaultValue;
}

inline bool  KeyValues::IsEmpty( int keySymbol )
{
	KeyValues *dat = FindKey( keySymbol );
	return dat ? dat->IsEmpty( ) : true;
}

inline char KeyValues::IsUsingEscapeSequences()
{
	return m_bHasEscapeSequences;
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
bool EvaluateConditional( const char *str );
#endif

#endif // KEYVALUES_H
