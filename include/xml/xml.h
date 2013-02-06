/*
 * Original code by Lee Thomason (www.grinninglizard.com)
 * Rewritten for mgz-utils by Grégoire Lejeune <gregoire.lejeune@free.fr>, (C) 2013
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 * not claim that you wrote the original software. If you use this
 * software in a product, an acknowledgment in the product documentation
 * would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */
#ifndef __MGZ_XML_XML_H
#define __MGZ_XML_XML_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Help out windows:
#if defined( _DEBUG ) && !defined( DEBUG )
#define DEBUG
#endif

#include <string>
#include <iostream>
#include <sstream>
#define MGZXML_STRING		std::string

#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
// Microsoft visual studio, version 2005 and higher.
#define MGZXML_SNPRINTF _snprintf_s
#define MGZXML_SSCANF   sscanf_s
#elif defined(_MSC_VER) && (_MSC_VER >= 1200 )
// Microsoft visual studio, version 6 and higher.
//#pragma message( "Using _sn* functions." )
#define MGZXML_SNPRINTF _snprintf
#define MGZXML_SSCANF   sscanf
#elif defined(__GNUC__) && (__GNUC__ >= 3 )
// GCC version 3 and higher.s
//#warning( "Using sn* functions." )
#define MGZXML_SNPRINTF snprintf
#define MGZXML_SSCANF   sscanf
#else
#define MGZXML_SNPRINTF snprintf
#define MGZXML_SSCANF   sscanf
#endif

namespace mgz {
  namespace xml {
    class document;
    class element;
    class comment;
    class unknown;
    class attribute;
    class text;
    class declaration;
    class parsingData;

    const int MGZXML_MAJOR_VERSION = 2;
    const int MGZXML_MINOR_VERSION = 6;
    const int MGZXML_PATCH_VERSION = 2;

    /*	Internal structure for tracking location of items 
        in the XML file.
        */
    struct cursor
    {
      cursor()		{ Clear(); }
      void Clear()		{ row = col = -1; }

      int row;	// 0 based.
      int col;	// 0 based.
    };


    /**
      Implements the interface to the "Visitor pattern" (see the Accept() method.)
      If you call the Accept() method, it requires being passed a visitor
      class to handle callbacks. For nodes that contain other nodes (Document, Element)
      you will get called with a VisitEnter/VisitExit pair. Nodes that are always leaves
      are simply called with Visit().

      If you return 'true' from a Visit method, recursive parsing will continue. If you return
      false, <b>no children of this node or its sibilings</b> will be Visited.

      All flavors of Visit methods have a default implementation that returns 'true' (continue 
      visiting). You need to only override methods that are interesting to you.

      Generally Accept() is called on the document, although all nodes suppert Visiting.

      You should never change the document from a callback.

      @sa node::Accept()
      */
    class visitor
    {
      public:
        virtual ~visitor() {}

        /// Visit a document.
        virtual bool VisitEnter( const document& /*doc*/ )			{ return true; }
        /// Visit a document.
        virtual bool VisitExit( const document& /*doc*/ )			{ return true; }

        /// Visit an element.
        virtual bool VisitEnter( const element& /*element*/, const attribute* /*firstAttribute*/ )	{ return true; }
        /// Visit an element.
        virtual bool VisitExit( const element& /*element*/ )		{ return true; }

        /// Visit a declaration
        virtual bool Visit( const declaration& /*declaration*/ )	{ return true; }
        /// Visit a text node
        virtual bool Visit( const text& /*text*/ )					{ return true; }
        /// Visit a comment node
        virtual bool Visit( const comment& /*comment*/ )			{ return true; }
        /// Visit an unknown node
        virtual bool Visit( const unknown& /*unknown*/ )			{ return true; }
    };

    // Only used by Attribute::Query functions
    enum 
    { 
      MGZXML_SUCCESS,
      MGZXML_NO_ATTRIBUTE,
      MGZXML_WRONG_TYPE
    };


    // Used by the parsing routines.
    enum encoding
    {
      MGZXML_ENCODING_UNKNOWN,
      MGZXML_ENCODING_UTF8,
      MGZXML_ENCODING_LEGACY
    };

    const encoding MGZXML_DEFAULT_ENCODING = MGZXML_ENCODING_UNKNOWN;

    /** base is a base class for every class in GlowXml.
      It does little except to establish that GlowXml classes
      can be printed and provide some utility functions.

      In XML, the document and elements can contain
      other elements and other types of nodes.

      @verbatim
      A Document can contain:	Element	(container or leaf)
      Comment (leaf)
      Unknown (leaf)
      Declaration( leaf )

      An Element can contain:	Element (container or leaf)
      Text	(leaf)
      Attributes (not on tree)
      Comment (leaf)
      Unknown (leaf)

      A Decleration contains: Attributes (not on tree)
      @endverbatim
      */
    class base
    {
      friend class node;
      friend class element;
      friend class document;

      public:
      base()	:	userData(0)		{}
      virtual ~base()			{}

      /**	All GlowXml classes can print themselves to a filestream
        or the string class (string in non-STL mode, std::string
        in STL mode.) Either or both cfile and str can be null.

        This is a formatted print, and will insert 
        tabs and newlines.

        (For an unformatted stream, use the << operator.)
        */
      virtual void Print( FILE* cfile, int depth ) const = 0;

      /**	The world does not agree on whether white space should be kept or
        not. In order to make everyone happy, these global, static functions
        are provided to set whether or not GlowXml will condense all white space
        into a single space or not. The default is to condense. Note changing this
        value is not thread safe.
        */
      static void SetCondenseWhiteSpace( bool condense )		{ condenseWhiteSpace = condense; }

      /// Return the current white space setting.
      static bool IsWhiteSpaceCondensed()						{ return condenseWhiteSpace; }

      /** Return the position, in the original source file, of this node or attribute.
        The row and column are 1-based. (That is the first row and first column is
        1,1). If the returns values are 0 or less, then the parser does not have
        a row and column value.

        Generally, the row and column value will be set when the document::Load(),
        document::LoadFile(), or any node::Parse() is called. It will NOT be set
        when the DOM was created from operator>>.

        The values reflect the initial load. Once the DOM is modified programmatically
        (by adding or changing nodes and attributes) the new values will NOT update to
        reflect changes in the document.

        There is a minor performance cost to computing the row and column. Computation
        can be disabled if document::SetTabSize() is called with 0 as the value.

        @sa document::SetTabSize()
        */
      int Row() const			{ return location.row + 1; }
      int Column() const		{ return location.col + 1; }	///< See Row()

      void  SetUserData( void* user )			{ userData = user; }	///< Set a pointer to arbitrary user data.
      void* GetUserData()						{ return userData; }	///< Get a pointer to arbitrary user data.
      const void* GetUserData() const 		{ return userData; }	///< Get a pointer to arbitrary user data.

      // Table that returs, for a given lead byte, the total number of bytes
      // in the UTF-8 sequence.
      static const int utf8ByteTable[256];

      virtual const char* Parse(	const char* p, 
          parsingData* data, 
          encoding xml_encoding /*= MGZXML_ENCODING_UNKNOWN */ ) = 0;

      /** Expands entities in a string. Note this should not contian the tag's '<', '>', etc, 
        or they will be transformed into entities!
        */
      static void EncodeString( const MGZXML_STRING& str, MGZXML_STRING* out );

      enum
      {
        MGZXML_NO_ERROR = 0,
        MGZXML_ERROR,
        MGZXML_ERROR_OPENING_FILE,
        MGZXML_ERROR_PARSING_ELEMENT,
        MGZXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
        MGZXML_ERROR_READING_ELEMENT_VALUE,
        MGZXML_ERROR_READING_ATTRIBUTES,
        MGZXML_ERROR_PARSING_EMPTY,
        MGZXML_ERROR_READING_END_TAG,
        MGZXML_ERROR_PARSING_UNKNOWN,
        MGZXML_ERROR_PARSING_COMMENT,
        MGZXML_ERROR_PARSING_DECLARATION,
        MGZXML_ERROR_DOCUMENT_EMPTY,
        MGZXML_ERROR_EMBEDDED_NULL,
        MGZXML_ERROR_PARSING_CDATA,
        MGZXML_ERROR_DOCUMENT_TOP_ONLY,

        MGZXML_ERROR_STRING_COUNT
      };

      protected:

      static const char* SkipWhiteSpace( const char*, encoding xml_encoding );

      inline static bool IsWhiteSpace( char c )		
      { 
        return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' ); 
      }
      inline static bool IsWhiteSpace( int c )
      {
        if ( c < 256 )
          return IsWhiteSpace( (char) c );
        return false;	// Again, only truly correct for English/Latin...but usually works.
      }

      static bool	StreamWhiteSpace( std::istream * in, MGZXML_STRING * tag );
      static bool StreamTo( std::istream * in, int character, MGZXML_STRING * tag );

      /*	Reads an XML name into the string provided. Returns
          a pointer just past the last character of the name,
          or 0 if the function has an error.
          */
      static const char* ReadName( const char* p, MGZXML_STRING* name, encoding xml_encoding );

      /*	Reads text. Returns a pointer past the given end tag.
          Wickedly complex options, but it keeps the (sensitive) code in one place.
          */
      static const char* ReadText(	const char* in,				// where to start
          MGZXML_STRING* text,			// the string read
          bool ignoreWhiteSpace,		// whether to keep the white space
          const char* endTag,			// what ends this text
          bool ignoreCase,			// whether to ignore case in the end tag
          encoding xml_encoding );	// the current encoding

      // If an entity has been found, transform it into a character.
      static const char* GetEntity( const char* in, char* value, int* length, encoding xml_encoding );

      // Get a character, while interpreting entities.
      // The length can be from 0 to 4 bytes.
      inline static const char* GetChar( const char* p, char* _value, int* length, encoding xml_encoding )
      {
        assert( p );
        if ( xml_encoding == MGZXML_ENCODING_UTF8 )
        {
          *length = utf8ByteTable[ *((const unsigned char*)p) ];
          assert( *length >= 0 && *length < 5 );
        }
        else
        {
          *length = 1;
        }

        if ( *length == 1 )
        {
          if ( *p == '&' )
            return GetEntity( p, _value, length, xml_encoding );
          *_value = *p;
          return p+1;
        }
        else if ( *length )
        {
          //strncpy( _value, p, *length );	// lots of compilers don't like this function (unsafe),
          // and the null terminator isn't needed
          for( int i=0; p[i] && i<*length; ++i ) {
            _value[i] = p[i];
          }
          return p + (*length);
        }
        else
        {
          // Not valid text.
          return 0;
        }
      }

      // Return true if the next characters in the stream are any of the endTag sequences.
      // Ignore case only works for english, and should only be relied on when comparing
      // to English words: StringEqual( p, "version", true ) is fine.
      static bool StringEqual(	const char* p,
          const char* endTag,
          bool ignoreCase,
          encoding xml_encoding );

      static const char* errorString[ MGZXML_ERROR_STRING_COUNT ];

      cursor location;

      /// Field containing a generic user pointer
      void*			userData;

      // None of these methods are reliable for any language except English.
      // Good for approximation, not great for accuracy.
      static int IsAlpha( unsigned char anyByte, encoding xml_encoding );
      static int IsAlphaNum( unsigned char anyByte, encoding xml_encoding );
      inline static int ToLower( int v, encoding xml_encoding )
      {
        if ( xml_encoding == MGZXML_ENCODING_UTF8 )
        {
          if ( v < 128 ) return tolower( v );
          return v;
        }
        else
        {
          return tolower( v );
        }
      }
      static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );

      private:
      base( const base& );				// not implemented.
      void operator=( const base& base );	// not allowed.

      struct Entity
      {
        const char*     str;
        unsigned int	strLength;
        char		    chr;
      };
      enum
      {
        NUM_ENTITY = 5,
        MAX_ENTITY_LENGTH = 6

      };
      static Entity entity[ NUM_ENTITY ];
      static bool condenseWhiteSpace;
    };


    /** The parent class for everything in the Document Object Model.
      (Except for attributes).
      Nodes have siblings, a parent, and children. A node can be
      in a document, or stand on its own. The type of a node
      can be queried, and it can be cast to its more defined type.
      */
    class node : public base
    {
      friend class document;
      friend class element;

      public:

      /** An input stream operator, for every class. Tolerant of newlines and
        formatting, but doesn't expect them.
        */
      friend std::istream& operator >> (std::istream& in, node& base);

      /** An output stream operator, for every class. Note that this outputs
        without any newlines or formatting, as opposed to Print(), which
        includes tabs and new lines.

        The operator<< and operator>> are not completely symmetric. Writing
        a node to a stream is very well defined. You'll get a nice stream
        of output, without any extra whitespace or newlines.

        But reading is not as well defined. (As it always is.) If you create
        a element (for example) and read that from an input stream,
        the text needs to define an element or junk will result. This is
        true of all input streams, but it's worth keeping in mind.

        A document will read nodes until it reads a root element, and
        all the children of that root element.
        */	
      friend std::ostream& operator<< (std::ostream& out, const node& base);

      /// Appends the XML node or attribute to a std::string.
      friend std::string& operator<< (std::string& out, const node& base );

      /** The types of XML nodes supported by GlowXml. (All the
        unsupported types are picked up by UNKNOWN.)
        */
      enum NodeType
      {
        MGZXML_DOCUMENT,
        MGZXML_ELEMENT,
        MGZXML_COMMENT,
        MGZXML_UNKNOWN,
        MGZXML_TEXT,
        MGZXML_DECLARATION,
        MGZXML_TYPECOUNT
      };

      virtual ~node();

      /** The meaning of 'value' changes for the specific type of
        node.
        @verbatim
Document:	filename of the xml file
Element:	name of the element
Comment:	the comment text
Unknown:	the tag contents
Text:		the text string
@endverbatim

The subclasses will wrap this function.
*/
      const char *Value() const { return value.c_str (); }

      /** Return Value() as a std::string. If you only use STL,
        this is more efficient than calling Value().
        Only available in STL mode.
        */
      const std::string& ValueStr() const { return value; }

      const MGZXML_STRING& ValueTStr() const { return value; }

      /** Changes the value of the node. Defined as:
        @verbatim
Document:	filename of the xml file
Element:	name of the element
Comment:	the comment text
Unknown:	the tag contents
Text:		the text string
@endverbatim
*/
      void SetValue(const char * _value) { value = _value;}

      /// STL std::string form.
      void SetValue( const std::string& _value )	{ value = _value; }

      /// Delete all the children of this node. Does not affect 'this'.
      void Clear();

      /// One step up the DOM.
      node* Parent()							{ return parent; }
      const node* Parent() const				{ return parent; }

      const node* FirstChild()	const		{ return firstChild; }	///< The first child of this node. Will be null if there are no children.
      node* FirstChild()						{ return firstChild; }
      const node* FirstChild( const char * value ) const;			///< The first child of this node with the matching 'value'. Will be null if none found.
      /// The first child of this node with the matching 'value'. Will be null if none found.
      node* FirstChild( const char * _value ) {
        // Call through to the const version - safe since nothing is changed. Exiting syntax: cast this to a const (always safe)
        // call the method, cast the return back to non-const.
        return const_cast< node* > ((const_cast< const node* >(this))->FirstChild( _value ));
      }
      const node* LastChild() const	{ return lastChild; }		/// The last child of this node. Will be null if there are no children.
      node* LastChild()	{ return lastChild; }

      const node* LastChild( const char * value ) const;			/// The last child of this node matching 'value'. Will be null if there are no children.
      node* LastChild( const char * _value ) {
        return const_cast< node* > ((const_cast< const node* >(this))->LastChild( _value ));
      }

      const node* FirstChild( const std::string& _value ) const	{	return FirstChild (_value.c_str ());	}	///< STL std::string form.
      node* FirstChild( const std::string& _value )				{	return FirstChild (_value.c_str ());	}	///< STL std::string form.
      const node* LastChild( const std::string& _value ) const	{	return LastChild (_value.c_str ());	}	///< STL std::string form.
      node* LastChild( const std::string& _value )				{	return LastChild (_value.c_str ());	}	///< STL std::string form.

      /** An alternate way to walk the children of a node.
        One way to iterate over nodes is:
        @verbatim
        for( child = parent->FirstChild(); child; child = child->NextSibling() )
        @endverbatim

        IterateChildren does the same thing with the syntax:
        @verbatim
        child = 0;
        while( child = parent->IterateChildren( child ) )
        @endverbatim

        IterateChildren takes the previous child as input and finds
        the next one. If the previous child is null, it returns the
        first. IterateChildren will return null when done.
        */
      const node* IterateChildren( const node* previous ) const;
      node* IterateChildren( const node* previous ) {
        return const_cast< node* >( (const_cast< const node* >(this))->IterateChildren( previous ) );
      }

      /// This flavor of IterateChildren searches for children with a particular 'value'
      const node* IterateChildren( const char * value, const node* previous ) const;
      node* IterateChildren( const char * _value, const node* previous ) {
        return const_cast< node* >( (const_cast< const node* >(this))->IterateChildren( _value, previous ) );
      }

      const node* IterateChildren( const std::string& _value, const node* previous ) const	{	return IterateChildren (_value.c_str (), previous);	}	///< STL std::string form.
      node* IterateChildren( const std::string& _value, const node* previous ) {	return IterateChildren (_value.c_str (), previous);	}	///< STL std::string form.

      /** Add a new node related to this. Adds a child past the LastChild.
        Returns a pointer to the new object or NULL if an error occured.
        */
      node* InsertEndChild( const node& addThis );


      /** Add a new node related to this. Adds a child past the LastChild.

NOTE: the node to be added is passed by pointer, and will be
henceforth owned (and deleted) by glowXml. This method is efficient
and avoids an extra copy, but should be used with care as it
uses a different memory model than the other insert functions.

@sa InsertEndChild
*/
      node* LinkEndChild( node* addThis );

      /** Add a new node related to this. Adds a child before the specified child.
        Returns a pointer to the new object or NULL if an error occured.
        */
      node* InsertBeforeChild( node* beforeThis, const node& addThis );

      /** Add a new node related to this. Adds a child after the specified child.
        Returns a pointer to the new object or NULL if an error occured.
        */
      node* InsertAfterChild(  node* afterThis, const node& addThis );

      /** Replace a child of this node.
        Returns a pointer to the new object or NULL if an error occured.
        */
      node* ReplaceChild( node* replaceThis, const node& withThis );

      /// Delete a child of this node.
      bool RemoveChild( node* removeThis );

      /// Navigate to a sibling node.
      const node* PreviousSibling() const			{ return prev; }
      node* PreviousSibling()						{ return prev; }

      /// Navigate to a sibling node.
      const node* PreviousSibling( const char * ) const;
      node* PreviousSibling( const char *_prev ) {
        return const_cast< node* >( (const_cast< const node* >(this))->PreviousSibling( _prev ) );
      }

      const node* PreviousSibling( const std::string& _value ) const	{	return PreviousSibling (_value.c_str ());	}	///< STL std::string form.
      node* PreviousSibling( const std::string& _value ) 			{	return PreviousSibling (_value.c_str ());	}	///< STL std::string form.
      const node* NextSibling( const std::string& _value) const		{	return NextSibling (_value.c_str ());	}	///< STL std::string form.
      node* NextSibling( const std::string& _value) 					{	return NextSibling (_value.c_str ());	}	///< STL std::string form.

      /// Navigate to a sibling node.
      const node* NextSibling() const				{ return next; }
      node* NextSibling()							{ return next; }

      /// Navigate to a sibling node with the given 'value'.
      const node* NextSibling( const char * ) const;
      node* NextSibling( const char* _next ) {
        return const_cast< node* >( (const_cast< const node* >(this))->NextSibling( _next ) );
      }

      /** Convenience function to get through elements.
        Calls NextSibling and ToElement. Will skip all non-Element
        nodes. Returns 0 if there is not another element.
        */
      const element* NextSiblingElement() const;
      element* NextSiblingElement() {
        return const_cast< element* >( (const_cast< const node* >(this))->NextSiblingElement() );
      }

      /** Convenience function to get through elements.
        Calls NextSibling and ToElement. Will skip all non-Element
        nodes. Returns 0 if there is not another element.
        */
      const element* NextSiblingElement( const char * ) const;
      element* NextSiblingElement( const char *_next ) {
        return const_cast< element* >( (const_cast< const node* >(this))->NextSiblingElement( _next ) );
      }

      const element* NextSiblingElement( const std::string& _value) const	{	return NextSiblingElement (_value.c_str ());	}	///< STL std::string form.
      element* NextSiblingElement( const std::string& _value)				{	return NextSiblingElement (_value.c_str ());	}	///< STL std::string form.

      /// Convenience function to get through elements.
      const element* FirstChildElement()	const;
      element* FirstChildElement() {
        return const_cast< element* >( (const_cast< const node* >(this))->FirstChildElement() );
      }

      /// Convenience function to get through elements.
      const element* FirstChildElement( const char * _value ) const;
      element* FirstChildElement( const char * _value ) {
        return const_cast< element* >( (const_cast< const node* >(this))->FirstChildElement( _value ) );
      }

      const element* FirstChildElement( const std::string& _value ) const	{	return FirstChildElement (_value.c_str ());	}	///< STL std::string form.
      element* FirstChildElement( const std::string& _value )				{	return FirstChildElement (_value.c_str ());	}	///< STL std::string form.

      /** Query the type (as an enumerated value, above) of this node.
        The possible types are: MGZXML_DOCUMENT, MGZXML_ELEMENT, MGZXML_COMMENT,
        MGZXML_UNKNOWN, MGZXML_TEXT, and MGZXML_DECLARATION.
        */
      int Type() const	{ return type; }

      /** Return a pointer to the Document this node lives in.
        Returns null if not in a document.
        */
      const document* GetDocument() const;
      document* GetDocument() {
        return const_cast< document* >( (const_cast< const node* >(this))->GetDocument() );
      }

      /// Returns true if this node has no children.
      bool NoChildren() const						{ return !firstChild; }

      virtual const document*    ToDocument()    const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual const element*     ToElement()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual const comment*     ToComment()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual const unknown*     ToUnknown()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual const text*        ToText()        const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual const declaration* ToDeclaration() const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

      virtual document*          ToDocument()    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual element*           ToElement()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual comment*           ToComment()     { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual unknown*           ToUnknown()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual text*	            ToText()        { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
      virtual declaration*       ToDeclaration() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

      /** Create an exact duplicate of this node and return it. The memory must be deleted
        by the caller. 
        */
      virtual node* Clone() const = 0;

      /** Accept a hierchical visit the nodes in the GlowXML DOM. Every node in the 
        XML tree will be conditionally visited and the host will be called back
        via the visitor interface.

        This is essentially a SAX interface for GlowXML. (Note however it doesn't re-parse
        the XML for the callbacks, so the performance of GlowXML is unchanged by using this
        interface versus any other.)

        The interface has been based on ideas from:

        - http://www.saxproject.org/
        - http://c2.com/cgi/wiki?HierarchicalVisitorPattern 

        Which are both good references for "visiting".

        An example of using Accept():
        @verbatim
        printer printer;
        glowxmlDoc.Accept( &printer );
        const char* xmlcstr = printer.CStr();
        @endverbatim
        */
      virtual bool Accept( visitor* visitor ) const = 0;

      protected:
      node( NodeType _type );

      // Copy to the allocated object. Shared functionality between Clone, Copy constructor,
      // and the assignment operator.
      void CopyTo( node* target ) const;

      // The real work of the input operator.
      virtual void StreamIn( std::istream* in, MGZXML_STRING* tag ) = 0;

      // Figure out what is at *p, and parse it. Returns null if it is not an xml node.
      node* Identify( const char* start, encoding xml_encoding );

      node*		parent;
      NodeType		type;

      node*		firstChild;
      node*		lastChild;

      MGZXML_STRING	value;

      node*		prev;
      node*		next;

      private:
      node( const node& );				// not implemented.
      void operator=( const node& base );	// not allowed.
    };


    /** An attribute is a name-value pair. Elements have an arbitrary
      number of attributes, each with a unique name.

      @note The attributes are not nodes, since they are not
      part of the glowXML document object model. There are other
      suggested ways to look at this problem.
      */
    class attribute : public base
    {
      friend class attributeSet;

      public:
      /// Construct an empty attribute.
      attribute() : base()
      {
        xml_document = 0;
        prev = next = 0;
      }

      /// std::string constructor.
      attribute( const std::string& _name, const std::string& _value )
      {
        name = _name;
        value = _value;
        xml_document = 0;
        prev = next = 0;
      }

      /// Construct an attribute with a name and value.
      attribute( const char * _name, const char * _value )
      {
        name = _name;
        value = _value;
        xml_document = 0;
        prev = next = 0;
      }

      const char*		Name()  const		{ return name.c_str(); }		///< Return the name of this attribute.
      const char*		Value() const		{ return value.c_str(); }		///< Return the value of this attribute.
      const std::string& ValueStr() const	{ return value; }				///< Return the value of this attribute.
      int				IntValue() const;									///< Return the value of this attribute, converted to an integer.
      double			DoubleValue() const;								///< Return the value of this attribute, converted to a double.

      // Get the glowxml string representation
      const MGZXML_STRING& NameTStr() const { return name; }

      /** QueryIntValue examines the value string. It is an alternative to the
        IntValue() method with richer error checking.
        If the value is an integer, it is stored in 'value' and 
        the call returns MGZXML_SUCCESS. If it is not
        an integer, it returns MGZXML_WRONG_TYPE.

        A specialized but useful call. Note that for success it returns 0,
        which is the opposite of almost all other GlowXml calls.
        */
      int QueryIntValue( int* _value ) const;
      /// QueryDoubleValue examines the value string. See QueryIntValue().
      int QueryDoubleValue( double* _value ) const;

      void SetName( const char* _name )	{ name = _name; }				///< Set the name of this attribute.
      void SetValue( const char* _value )	{ value = _value; }				///< Set the value.

      void SetIntValue( int _value );										///< Set the value from an integer.
      void SetDoubleValue( double _value );								///< Set the value from a double.

      /// STL std::string form.
      void SetName( const std::string& _name )	{ name = _name; }	
      /// STL std::string form.	
      void SetValue( const std::string& _value )	{ value = _value; }

      /// Get the next sibling attribute in the DOM. Returns null at end.
      const attribute* Next() const;
      attribute* Next() {
        return const_cast< attribute* >( (const_cast< const attribute* >(this))->Next() ); 
      }

      /// Get the previous sibling attribute in the DOM. Returns null at beginning.
      const attribute* Previous() const;
      attribute* Previous() {
        return const_cast< attribute* >( (const_cast< const attribute* >(this))->Previous() ); 
      }

      bool operator==( const attribute& rhs ) const { return rhs.name == name; }
      bool operator<( const attribute& rhs )	 const { return name < rhs.name; }
      bool operator>( const attribute& rhs )  const { return name > rhs.name; }

      /*	Attribute parsing starts: first letter of the name
returns: the next char after the value end quote
*/
      virtual const char* Parse( const char* p, parsingData* data, encoding xml_encoding );

      // Prints this Attribute to a FILE stream.
      virtual void Print( FILE* cfile, int depth ) const {
        Print( cfile, depth, 0 );
      }
      void Print( FILE* cfile, int depth, MGZXML_STRING* str ) const;

      // [internal use]
      // Set the document pointer so the attribute can report errors.
      void SetDocument( document* doc )	{ xml_document = doc; }

      private:
      attribute( const attribute& );				// not implemented.
      void operator=( const attribute& base );	// not allowed.

      document*	xml_document;	// A pointer back to a document, for error reporting.
      MGZXML_STRING name;
      MGZXML_STRING value;
      attribute*	prev;
      attribute*	next;
    };


    /*	A class used to manage a group of attributes.
        It is only used internally, both by the ELEMENT and the DECLARATION.

        The set can be changed transparent to the Element and Declaration
        classes that use it, but NOT transparent to the Attribute
        which has to implement a next() and previous() method. Which makes
        it a bit problematic and prevents the use of STL.

        This version is implemented with circular lists because:
        - I like circular lists
        - it demonstrates some independence from the (typical) doubly linked list.
        */
    class attributeSet
    {
      public:
        attributeSet();
        ~attributeSet();

        void Add( attribute* attribute );
        void Remove( attribute* attribute );

        const attribute* First()	const	{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
        attribute* First()					{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
        const attribute* Last() const		{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
        attribute* Last()					{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }

        attribute*	Find( const char* _name ) const;
        attribute* FindOrCreate( const char* _name );

        attribute*	Find( const std::string& _name ) const;
        attribute* FindOrCreate( const std::string& _name );


      private:
        //*ME:	Because of hidden/disabled copy-construktor in attribute (sentinel-element),
        //*ME:	this class must be also use a hidden/disabled copy-constructor !!!
        attributeSet( const attributeSet& );	// not allowed
        void operator=( const attributeSet& );	// not allowed (as attribute)

        attribute sentinel;
    };


    /** The element is a container class. It has a value, the element name,
      and can contain other elements, text, comments, and unknowns.
      Elements also contain an arbitrary number of attributes.
      */
    class element : public node
    {
      public:
        /// Construct an element.
        element (const char * in_value);

        /// std::string constructor.
        element( const std::string& _value );

        element( const element& );

        element& operator=( const element& base );

        virtual ~element();

        /** Given an attribute name, Attribute() returns the value
          for the attribute of that name, or null if none exists.
          */
        const char* Attribute( const char* name ) const;

        /** Given an attribute name, Attribute() returns the value
          for the attribute of that name, or null if none exists.
          If the attribute exists and can be converted to an integer,
          the integer value will be put in the return 'i', if 'i'
          is non-null.
          */
        const char* Attribute( const char* name, int* i ) const;

        /** Given an attribute name, Attribute() returns the value
          for the attribute of that name, or null if none exists.
          If the attribute exists and can be converted to an double,
          the double value will be put in the return 'd', if 'd'
          is non-null.
          */
        const char* Attribute( const char* name, double* d ) const;

        /** QueryIntAttribute examines the attribute - it is an alternative to the
          Attribute() method with richer error checking.
          If the attribute is an integer, it is stored in 'value' and 
          the call returns MGZXML_SUCCESS. If it is not
          an integer, it returns MGZXML_WRONG_TYPE. If the attribute
          does not exist, then MGZXML_NO_ATTRIBUTE is returned.
          */	
        int QueryIntAttribute( const char* name, int* _value ) const;
        /// QueryUnsignedAttribute examines the attribute - see QueryIntAttribute().
        int QueryUnsignedAttribute( const char* name, unsigned* _value ) const;
        /** QueryBoolAttribute examines the attribute - see QueryIntAttribute(). 
          Note that '1', 'true', or 'yes' are considered true, while '0', 'false'
          and 'no' are considered false.
          */
        int QueryBoolAttribute( const char* name, bool* _value ) const;
        /// QueryDoubleAttribute examines the attribute - see QueryIntAttribute().
        int QueryDoubleAttribute( const char* name, double* _value ) const;
        /// QueryFloatAttribute examines the attribute - see QueryIntAttribute().
        int QueryFloatAttribute( const char* name, float* _value ) const {
          double d;
          int result = QueryDoubleAttribute( name, &d );
          if ( result == MGZXML_SUCCESS ) {
            *_value = (float)d;
          }
          return result;
        }

        /// QueryStringAttribute examines the attribute - see QueryIntAttribute().
        int QueryStringAttribute( const char* name, std::string* _value ) const {
          const char* cstr = Attribute( name );
          if ( cstr ) {
            *_value = std::string( cstr );
            return MGZXML_SUCCESS;
          }
          return MGZXML_NO_ATTRIBUTE;
        }

        /** Template form of the attribute query which will try to read the
          attribute into the specified type. Very easy, very powerful, but
          be careful to make sure to call this with the correct type.

NOTE: This method doesn't work correctly for 'string' types that contain spaces.

@return MGZXML_SUCCESS, MGZXML_WRONG_TYPE, or MGZXML_NO_ATTRIBUTE
*/
        template< typename T > int QueryValueAttribute( const std::string& name, T* outValue ) const
        {
          const attribute* xml_node = xml_attributeSet.Find( name );
          if ( !xml_node )
            return MGZXML_NO_ATTRIBUTE;

          std::stringstream sstream( xml_node->ValueStr() );
          sstream >> *outValue;
          if ( !sstream.fail() )
            return MGZXML_SUCCESS;
          return MGZXML_WRONG_TYPE;
        }

        int QueryValueAttribute( const std::string& name, std::string* outValue ) const
        {
          const attribute* xml_node = xml_attributeSet.Find( name );
          if ( !xml_node )
            return MGZXML_NO_ATTRIBUTE;
          *outValue = xml_node->ValueStr();
          return MGZXML_SUCCESS;
        }

        /** Sets an attribute of name to a given value. The attribute
          will be created if it does not exist, or changed if it does.
          */
        void SetAttribute( const char* name, const char * _value );

        const std::string* Attribute( const std::string& name ) const;
        const std::string* Attribute( const std::string& name, int* i ) const;
        const std::string* Attribute( const std::string& name, double* d ) const;
        int QueryIntAttribute( const std::string& name, int* _value ) const;
        int QueryDoubleAttribute( const std::string& name, double* _value ) const;

        /// STL std::string form.
        void SetAttribute( const std::string& name, const std::string& _value );
        ///< STL std::string form.
        void SetAttribute( const std::string& name, int _value );
        ///< STL std::string form.
        void SetDoubleAttribute( const std::string& name, double value );

        /** Sets an attribute of name to a given value. The attribute
          will be created if it does not exist, or changed if it does.
          */
        void SetAttribute( const char * name, int value );

        /** Sets an attribute of name to a given value. The attribute
          will be created if it does not exist, or changed if it does.
          */
        void SetDoubleAttribute( const char * name, double value );

        /** Deletes an attribute with the given name.
        */
        void RemoveAttribute( const char * name );
        void RemoveAttribute( const std::string& name )	{	RemoveAttribute (name.c_str ());	}	///< STL std::string form.

        const attribute* FirstAttribute() const	{ return xml_attributeSet.First(); }		///< Access the first attribute in this element.
        attribute* FirstAttribute() 				{ return xml_attributeSet.First(); }
        const attribute* LastAttribute()	const 	{ return xml_attributeSet.Last(); }		///< Access the last attribute in this element.
        attribute* LastAttribute()					{ return xml_attributeSet.Last(); }

        /** Convenience function for easy access to the text inside an element. Although easy
          and concise, GetText() is limited compared to getting the text child
          and accessing it directly.

          If the first child of 'this' is a text, the GetText()
          returns the character string of the Text node, else null is returned.

          This is a convenient method for getting the text of simple contained text:
          @verbatim
          <foo>This is text</foo>
          const char* str = fooElement->GetText();
          @endverbatim

          'str' will be a pointer to "This is text". 

          Note that this function can be misleading. If the element foo was created from
          this XML:
          @verbatim
          <foo><b>This is text</b></foo> 
          @endverbatim

          then the value of str would be null. The first child node isn't a text node, it is
          another element. From this XML:
          @verbatim
          <foo>This is <b>text</b></foo> 
          @endverbatim
          GetText() will return "This is ".

WARNING: GetText() accesses a child node - don't become confused with the 
similarly named handle::Text() and node::ToText() which are 
safe type casts on the referenced node.
*/
        const char* GetText() const;

        /// Creates a new Element and returns it - the returned element is a copy.
        virtual node* Clone() const;
        // Print the Element to a FILE stream.
        virtual void Print( FILE* cfile, int depth ) const;

        /*	Attribtue parsing starts: next char past '<'
returns: next char past '>'
*/
        virtual const char* Parse( const char* p, parsingData* data, encoding xml_encoding );

        virtual const element*     ToElement()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
        virtual element*           ToElement()	          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

        /** Walk the XML tree visiting this node and all of its children. 
        */
        virtual bool Accept( visitor* visitor ) const;

      protected:

        void CopyTo( element* target ) const;
        void ClearThis();	// like clear, but initializes 'this' object as well

        // Used to be public [internal use]
        virtual void StreamIn( std::istream * in, MGZXML_STRING * tag );
        /*	[internal use]
            Reads the "value" of the element -- another element, or text.
            This should terminate with the current end tag.
            */
        const char* ReadValue( const char* in, parsingData* prevData, encoding xml_encoding );

      private:
        attributeSet xml_attributeSet;
    };


    /**	An XML comment.
    */
    class comment : public node
    {
      public:
        /// Constructs an empty comment.
        comment() : node( node::MGZXML_COMMENT ) {}
        /// Construct a comment from text.
        comment( const char* _value ) : node( node::MGZXML_COMMENT ) {
          SetValue( _value );
        }
        comment( const comment& );
        comment& operator=( const comment& base );

        virtual ~comment()	{}

        /// Returns a copy of this Comment.
        virtual node* Clone() const;
        // Write this Comment to a FILE stream.
        virtual void Print( FILE* cfile, int depth ) const;

        /*	Attribtue parsing starts: at the ! of the !--
returns: next char past '>'
*/
        virtual const char* Parse( const char* p, parsingData* data, encoding xml_encoding );

        virtual const comment*  ToComment() const	{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.
        virtual		  comment*  ToComment()		{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.

        /** Walk the XML tree visiting this node and all of its children. 
        */
        virtual bool Accept( visitor* visitor ) const;

      protected:
        void CopyTo( comment* target ) const;

        // used to be public
        virtual void StreamIn( std::istream * in, MGZXML_STRING * tag );
        //	virtual void StreamOut( MGZXML_OSTREAM * out ) const;

      private:

    };


    /** XML text. A text node can have 2 ways to output the next. "normal" output 
      and CDATA. It will default to the mode it was parsed from the XML file and
      you generally want to leave it alone, but you can change the output mode with 
      SetCDATA() and query it with CDATA().
      */
    class text : public node
    {
      friend class element;
      public:
      /** Constructor for text element. By default, it is treated as 
        normal, encoded text. If you want it be output as a CDATA text
        element, set the parameter _cdata to 'true'
        */
      text (const char * initValue ) : node (node::MGZXML_TEXT)
      {
        SetValue( initValue );
        cdata = false;
      }
      virtual ~text() {}

      /// Constructor.
      text( const std::string& initValue ) : node (node::MGZXML_TEXT)
      {
        SetValue( initValue );
        cdata = false;
      }

      text( const text& copy ) : node( node::MGZXML_TEXT )	{ copy.CopyTo( this ); }
      text& operator=( const text& base )							 	{ base.CopyTo( this ); return *this; }

      // Write this text object to a FILE stream.
      virtual void Print( FILE* cfile, int depth ) const;

      /// Queries whether this represents text using a CDATA section.
      bool CDATA() const				{ return cdata; }
      /// Turns on or off a CDATA representation of text.
      void SetCDATA( bool _cdata )	{ cdata = _cdata; }

      virtual const char* Parse( const char* p, parsingData* data, encoding xml_encoding );

      virtual const text* ToText() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
      virtual text*       ToText()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

      /** Walk the XML tree visiting this node and all of its children. 
      */
      virtual bool Accept( visitor* content ) const;

      protected :
      ///  [internal use] Creates a new Element and returns it.
      virtual node* Clone() const;
      void CopyTo( text* target ) const;

      bool Blank() const;	// returns true if all white space and new lines
      // [internal use]
      virtual void StreamIn( std::istream * in, MGZXML_STRING * tag );

      private:
      bool cdata;			// true if this should be input and output as a CDATA style text element
    };


    /** In correct XML the declaration is the first entry in the file.
      @verbatim
      <?xml version="1.0" standalone="yes"?>
      @endverbatim

      GlowXml will happily read or write files without a declaration,
      however. There are 3 possible attributes to the declaration:
      version, encoding, and standalone.

Note: In this version of the code, the attributes are
handled as special cases, not generic attributes, simply
because there can only be at most 3 and they are always the same.
*/
    class declaration : public node
    {
      public:
        /// Construct an empty declaration.
        declaration()   : node( node::MGZXML_DECLARATION ) {}

        /// Constructor.
        declaration(	const std::string& _version,
            const std::string& _xml_encoding,
            const std::string& _standalone );

        /// Construct.
        declaration(	const char* _version,
            const char* _xml_encoding,
            const char* _standalone );

        declaration( const declaration& copy );
        declaration& operator=( const declaration& copy );

        virtual ~declaration()	{}

        /// Version. Will return an empty string if none was found.
        const char *Version() const			{ return version.c_str (); }
        /// Encoding. Will return an empty string if none was found.
        const char *Encoding() const		{ return xml_encoding.c_str (); }
        /// Is this a standalone document?
        const char *Standalone() const		{ return standalone.c_str (); }

        /// Creates a copy of this Declaration and returns it.
        virtual node* Clone() const;
        // Print this declaration to a FILE stream.
        virtual void Print( FILE* cfile, int depth, MGZXML_STRING* str ) const;
        virtual void Print( FILE* cfile, int depth ) const {
          Print( cfile, depth, 0 );
        }

        virtual const char* Parse( const char* p, parsingData* data, encoding xml_encoding );

        virtual const declaration* ToDeclaration() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
        virtual declaration*       ToDeclaration()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

        /** Walk the XML tree visiting this node and all of its children. 
        */
        virtual bool Accept( visitor* visitor ) const;

      protected:
        void CopyTo( declaration* target ) const;
        // used to be public
        virtual void StreamIn( std::istream * in, MGZXML_STRING * tag );

      private:

        MGZXML_STRING version;
        MGZXML_STRING xml_encoding;
        MGZXML_STRING standalone;
    };


    /** Any tag that glowXml doesn't recognize is saved as an
      unknown. It is a tag of text, but should not be modified.
      It will be written back to the XML, unchanged, when the file
      is saved.

      DTD tags get thrown into unknowns.
      */
    class unknown : public node
    {
      public:
        unknown() : node( node::MGZXML_UNKNOWN )	{}
        virtual ~unknown() {}

        unknown( const unknown& copy ) : node( node::MGZXML_UNKNOWN )		{ copy.CopyTo( this ); }
        unknown& operator=( const unknown& copy )										{ copy.CopyTo( this ); return *this; }

        /// Creates a copy of this Unknown and returns it.
        virtual node* Clone() const;
        // Print this Unknown to a FILE stream.
        virtual void Print( FILE* cfile, int depth ) const;

        virtual const char* Parse( const char* p, parsingData* data, encoding xml_encoding );

        virtual const unknown*     ToUnknown()     const	{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.
        virtual unknown*           ToUnknown()				{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.

        /** Walk the XML tree visiting this node and all of its children. 
        */
        virtual bool Accept( visitor* content ) const;

      protected:
        void CopyTo( unknown* target ) const;

        virtual void StreamIn( std::istream * in, MGZXML_STRING * tag );

      private:

    };


    /** Always the top level node. A document binds together all the
      XML pieces. It can be saved, loaded, and printed to the screen.
      The 'value' of a document node is the xml file name.
      */
    class document : public node
    {
      public:
        /// Create an empty document, that has no name.
        document();
        /// Create a document with a name. The name of the document is also the filename of the xml.
        document( const char * documentName );

        /// Constructor.
        document( const std::string& documentName );

        document( const document& copy );
        document& operator=( const document& copy );

        virtual ~document() {}

        /** Load a file using the current document value.
          Returns true if successful. Will delete any existing
          document data before loading.
          */
        bool LoadFile( encoding xml_encoding = MGZXML_DEFAULT_ENCODING );
        /// Save a file using the current document value. Returns true if successful.
        bool SaveFile() const;
        /// Load a file using the given filename. Returns true if successful.
        bool LoadFile( const char * filename, encoding xml_encoding = MGZXML_DEFAULT_ENCODING );
        /// Save a file using the given filename. Returns true if successful.
        bool SaveFile( const char * filename ) const;
        /** Load a file using the given FILE*. Returns true if successful. Note that this method
          doesn't stream - the entire object pointed at by the FILE*
          will be interpreted as an XML file. GlowXML doesn't stream in XML from the current
          file location. Streaming may be added in the future.
          */
        bool LoadFile( FILE*, encoding xml_encoding = MGZXML_DEFAULT_ENCODING );
        /// Save a file using the given FILE*. Returns true if successful.
        bool SaveFile( FILE* ) const;

        bool LoadFile( const std::string& filename, encoding xml_encoding = MGZXML_DEFAULT_ENCODING )			///< STL std::string version.
        {
          return LoadFile( filename.c_str(), xml_encoding );
        }
        bool SaveFile( const std::string& filename ) const		///< STL std::string version.
        {
          return SaveFile( filename.c_str() );
        }

        /** Parse the given null terminated block of xml data. Passing in an encoding to this
          method (either MGZXML_ENCODING_LEGACY or MGZXML_ENCODING_UTF8 will force GlowXml
          to use that encoding, regardless of what GlowXml might otherwise try to detect.
          */
        virtual const char* Parse( const char* p, parsingData* data = 0, encoding xml_encoding = MGZXML_DEFAULT_ENCODING );

        /** Get the root element -- the only top level element -- of the document.
          In well formed XML, there should only be one. GlowXml is tolerant of
          multiple elements at the document level.
          */
        const element* RootElement() const		{ return FirstChildElement(); }
        element* RootElement()					{ return FirstChildElement(); }

        /** If an error occurs, Error will be set to true. Also,
          - The ErrorId() will contain the integer identifier of the error (not generally useful)
          - The ErrorDesc() method will return the name of the error. (very useful)
          - The ErrorRow() and ErrorCol() will return the location of the error (if known)
          */	
        bool Error() const						{ return error; }

        /// Contains a textual (english) description of the error if one occurs.
        const char * ErrorDesc() const	{ return errorDesc.c_str (); }

        /** Generally, you probably want the error string ( ErrorDesc() ). But if you
          prefer the ErrorId, this function will fetch it.
          */
        int ErrorId()	const				{ return errorId; }

        /** Returns the location (if known) of the error. The first column is column 1, 
          and the first row is row 1. A value of 0 means the row and column wasn't applicable
          (memory errors, for example, have no row/column) or the parser lost the error. (An
          error in the error reporting, in that case.)

          @sa SetTabSize, Row, Column
          */
        int ErrorRow() const	{ return errorLocation.row+1; }
        int ErrorCol() const	{ return errorLocation.col+1; }	///< The column where the error occured. See ErrorRow()

        /** SetTabSize() allows the error reporting functions (ErrorRow() and ErrorCol())
          to report the correct values for row and column. It does not change the output
          or input in any way.

          By calling this method, with a tab size
          greater than 0, the row and column of each node and attribute is stored
          when the file is loaded. Very useful for tracking the DOM back in to
          the source file.

          The tab size is required for calculating the location of nodes. If not
          set, the default of 4 is used. The tabsize is set per document. Setting
          the tabsize to 0 disables row/column tracking.

          Note that row and column tracking is not supported when using operator>>.

          The tab size needs to be enabled before the parse or load. Correct usage:
          @verbatim
          document doc;
          doc.SetTabSize( 8 );
          doc.Load( "myfile.xml" );
          @endverbatim

          @sa Row, Column
          */
        void SetTabSize( int _tabsize )		{ tabsize = _tabsize; }

        int TabSize() const	{ return tabsize; }

        /** If you have handled the error, it can be reset with this call. The error
          state is automatically cleared if you Parse a new XML block.
          */
        void ClearError()						{	error = false; 
          errorId = 0; 
          errorDesc = ""; 
          errorLocation.row = errorLocation.col = 0; 
          //errorLocation.last = 0; 
        }

        /** Write the document to standard out using formatted printing ("pretty print"). */
        void Print() const						{ Print( stdout, 0 ); }

        /* Write the document to a string using formatted printing ("pretty print"). This
           will allocate a character array (new char[]) and return it as a pointer. The
           calling code pust call delete[] on the return char* to avoid a memory leak.
           */
        //char* PrintToMemory() const; 

        /// Print this Document to a FILE stream.
        virtual void Print( FILE* cfile, int depth = 0 ) const;
        // [internal use]
        void SetError( int err, const char* errorLocation, parsingData* prevData, encoding xml_encoding );

        virtual const document*    ToDocument()    const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
        virtual document*          ToDocument()          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

        /** Walk the XML tree visiting this node and all of its children. 
        */
        virtual bool Accept( visitor* content ) const;

      protected :
        // [internal use]
        virtual node* Clone() const;
        virtual void StreamIn( std::istream * in, MGZXML_STRING * tag );

      private:
        void CopyTo( document* target ) const;

        bool error;
        int  errorId;
        MGZXML_STRING errorDesc;
        int tabsize;
        cursor errorLocation;
        bool useMicrosoftBOM;		// the UTF-8 BOM were found when read. Note this, and try to write.
    };


    /**
      A handle is a class that wraps a node pointer with null checks; this is
      an incredibly useful thing. Note that handle is not part of the GlowXml
      DOM structure. It is a separate utility class.

      Take an example:
      @verbatim
      <Document>
      <Element attributeA = "valueA">
      <Child attributeB = "value1" />
      <Child attributeB = "value2" />
      </Element>
      <Document>
      @endverbatim

      Assuming you want the value of "attributeB" in the 2nd "Child" element, it's very 
      easy to write a *lot* of code that looks like:

      @verbatim
      element* root = document.FirstChildElement( "Document" );
      if ( root )
      {
      element* element = root->FirstChildElement( "Element" );
      if ( element )
      {
      element* child = element->FirstChildElement( "Child" );
      if ( child )
      {
      element* child2 = child->NextSiblingElement( "Child" );
      if ( child2 )
      {
    // Finally do something useful.
    @endverbatim

    And that doesn't even cover "else" cases. handle addresses the verbosity
    of such code. A handle checks for null	pointers so it is perfectly safe 
    and correct to use:

    @verbatim
    handle docHandle( &document );
    element* child2 = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", 1 ).ToElement();
    if ( child2 )
    {
    // do something useful
    @endverbatim

    Which is MUCH more concise and useful.

    It is also safe to copy handles - internally they are nothing more than node pointers.
    @verbatim
    handle handleCopy = handle;
    @endverbatim

    What they should not be used for is iteration:

    @verbatim
    int i=0; 
    while ( true )
    {
    element* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", i ).ToElement();
    if ( !child )
    break;
    // do something
    ++i;
    }
    @endverbatim

    It seems reasonable, but it is in fact two embedded while loops. The Child method is 
    a linear walk to find the element, so this code would iterate much more than it needs 
    to. Instead, prefer:

    @verbatim
      element* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).FirstChild( "Child" ).ToElement();

    for( child; child; child=child->NextSiblingElement() )
    {
      // do something
    }
    @endverbatim
      */
      class handle
      {
        public:
          /// Create a handle from any node (at any depth of the tree.) This can be a null pointer.
          handle( node* _xml_node )					{ this->xml_node = _xml_node; }
          /// Copy constructor
          handle( const handle& ref )			{ this->xml_node = ref.xml_node; }
          handle operator=( const handle& ref ) { if ( &ref != this ) this->xml_node = ref.xml_node; return *this; }

          /// Return a handle to the first child node.
          handle FirstChild() const;
          /// Return a handle to the first child node with the given name.
          handle FirstChild( const char * value ) const;
          /// Return a handle to the first child element.
          handle FirstChildElement() const;
          /// Return a handle to the first child element with the given name.
          handle FirstChildElement( const char * value ) const;

          /** Return a handle to the "index" child with the given name. 
            The first child is 0, the second 1, etc.
            */
          handle Child( const char* value, int index ) const;
          /** Return a handle to the "index" child. 
            The first child is 0, the second 1, etc.
            */
          handle Child( int index ) const;
          /** Return a handle to the "index" child element with the given name. 
            The first child element is 0, the second 1, etc. Note that only elements
            are indexed: other types are not counted.
            */
          handle ChildElement( const char* value, int index ) const;
          /** Return a handle to the "index" child element. 
            The first child element is 0, the second 1, etc. Note that only elements
            are indexed: other types are not counted.
            */
          handle ChildElement( int index ) const;

          handle FirstChild( const std::string& _value ) const				{ return FirstChild( _value.c_str() ); }
          handle FirstChildElement( const std::string& _value ) const		{ return FirstChildElement( _value.c_str() ); }

          handle Child( const std::string& _value, int index ) const			{ return Child( _value.c_str(), index ); }
          handle ChildElement( const std::string& _value, int index ) const	{ return ChildElement( _value.c_str(), index ); }

          /** Return the handle as a node. This may return null.
          */
          node* ToNode() const			{ return xml_node; } 
          /** Return the handle as a element. This may return null.
          */
          element* ToElement() const		{ return ( ( xml_node && xml_node->ToElement() ) ? xml_node->ToElement() : 0 ); }
          /**	Return the handle as a text. This may return null.
          */
          text* ToText() const			{ return ( ( xml_node && xml_node->ToText() ) ? xml_node->ToText() : 0 ); }
          /** Return the handle as a unknown. This may return null.
          */
          unknown* ToUnknown() const		{ return ( ( xml_node && xml_node->ToUnknown() ) ? xml_node->ToUnknown() : 0 ); }

          /** @deprecated use ToNode. 
            Return the handle as a node. This may return null.
            */
          node* Node() const			{ return ToNode(); } 
          /** @deprecated use ToElement. 
            Return the handle as a element. This may return null.
            */
          element* Element() const	{ return ToElement(); }
          /**	@deprecated use ToText()
            Return the handle as a text. This may return null.
            */
          text* Text() const			{ return ToText(); }
          /** @deprecated use ToUnknown()
            Return the handle as a unknown. This may return null.
            */
          unknown* Unknown() const	{ return ToUnknown(); }

        private:
          node* xml_node;
      };


    /** Print to memory functionality. The printer is useful when you need to:

      -# Print to memory (especially in non-STL mode)
      -# Control formatting (line endings, etc.)

      When constructed, the printer is in its default "pretty printing" mode.
      Before calling Accept() you can call methods to control the printing
      of the XML document. After node::Accept() is called, the printed document can
      be accessed via the CStr(), Str(), and Size() methods.

      printer uses the Visitor API.
      @verbatim
      printer printer;
      printer.SetIndent( "\t" );

      doc.Accept( &printer );
      fprintf( stdout, "%s", printer.CStr() );
      @endverbatim
      */
    class printer : public visitor
    {
      public:
        printer() : depth( 0 ), simpleTextPrint( false ),
        buffer(), indent( "    " ), lineBreak( "\n" ) {}

        virtual bool VisitEnter( const document& doc );
        virtual bool VisitExit( const document& doc );

        virtual bool VisitEnter( const element& element, const attribute* firstAttribute );
        virtual bool VisitExit( const element& element );

        virtual bool Visit( const declaration& declaration );
        virtual bool Visit( const text& text );
        virtual bool Visit( const comment& comment );
        virtual bool Visit( const unknown& unknown );

        /** Set the indent characters for printing. By default 4 spaces
          but tab (\t) is also useful, or null/empty string for no indentation.
          */
        void SetIndent( const char* _indent )			{ indent = _indent ? _indent : "" ; }
        /// Query the indention string.
        const char* Indent()							{ return indent.c_str(); }
        /** Set the line breaking string. By default set to newline (\n). 
          Some operating systems prefer other characters, or can be
          set to the null/empty string for no indenation.
          */
        void SetLineBreak( const char* _lineBreak )		{ lineBreak = _lineBreak ? _lineBreak : ""; }
        /// Query the current line breaking string.
        const char* LineBreak()							{ return lineBreak.c_str(); }

        /** Switch over to "stream printing" which is the most dense formatting without 
          linebreaks. Common when the XML is needed for network transmission.
          */
        void SetStreamPrinting()						{ indent = "";
          lineBreak = "";
        }	
        /// Return the result.
        const char* CStr()								{ return buffer.c_str(); }
        /// Return the length of the result string.
        size_t Size()									{ return buffer.size(); }

        /// Return the result.
        const std::string& Str()						{ return buffer; }

      private:
        void DoIndent()	{
          for( int i=0; i<depth; ++i )
            buffer += indent;
        }
        void DoLineBreak() {
          buffer += lineBreak;
        }

        int depth;
        bool simpleTextPrint;
        MGZXML_STRING buffer;
        MGZXML_STRING indent;
        MGZXML_STRING lineBreak;
    };

  }
  }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
