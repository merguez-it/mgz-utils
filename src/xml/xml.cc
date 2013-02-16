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
#include <ctype.h>

#include <sstream>
#include <iostream>

#include "xml/xml.h"

namespace mgz {
  namespace xml {
    FILE* fOpen( const char* filename, const char* mode );

    bool base::condenseWhiteSpace = true;

    // Microsoft compiler security
    FILE* fOpen( const char* filename, const char* mode )
    {
#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
      FILE* fp = 0;
      errno_t err = fopen_s( &fp, filename, mode );
      if ( !err && fp )
        return fp;
      return 0;
#else
      return fopen( filename, mode );
#endif
    }

    void base::EncodeString( const MGZXML_STRING& str, MGZXML_STRING* outString )
    {
      int i=0;

      while( i<(int)str.length() )
      {
        unsigned char c = (unsigned char) str[i];

        if (    c == '&' 
            && i < ( (int)str.length() - 2 )
            && str[i+1] == '#'
            && str[i+2] == 'x' )
        {
          // Hexadecimal character reference.
          // Pass through unchanged.
          // &#xA9;	-- copyright symbol, for example.
          //
          // The -1 is a bug fix from Rob Laveaux. It keeps
          // an overflow from happening if there is no ';'.
          // There are actually 2 ways to exit this loop -
          // while fails (error case) and break (semicolon found).
          // However, there is no mechanism (currently) for
          // this function to return an error.
          while ( i<(int)str.length()-1 )
          {
            outString->append( str.c_str() + i, 1 );
            ++i;
            if ( str[i] == ';' )
              break;
          }
        }
        else if ( c == '&' )
        {
          outString->append( entity[0].str, entity[0].strLength );
          ++i;
        }
        else if ( c == '<' )
        {
          outString->append( entity[1].str, entity[1].strLength );
          ++i;
        }
        else if ( c == '>' )
        {
          outString->append( entity[2].str, entity[2].strLength );
          ++i;
        }
        else if ( c == '\"' )
        {
          outString->append( entity[3].str, entity[3].strLength );
          ++i;
        }
        else if ( c == '\'' )
        {
          outString->append( entity[4].str, entity[4].strLength );
          ++i;
        }
        else if ( c < 32 )
        {
          // Easy pass at non-alpha/numeric/symbol
          // Below 32 is symbolic.
          char buf[ 32 ];

#if defined(MGZXML_SNPRINTF)		
          MGZXML_SNPRINTF( buf, sizeof(buf), "&#x%02X;", (unsigned) ( c & 0xff ) );
#else
          sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
#endif		

          //*ME:	warning C4267: convert 'size_t' to 'int'
          //*ME:	Int-Cast to make compiler happy ...
          outString->append( buf, (int)strlen( buf ) );
          ++i;
        }
        else
        {
          //char realc = (char) c;
          //outString->append( &realc, 1 );
          *outString += (char) c;	// somewhat more efficient function call.
          ++i;
        }
      }
    }


    node::node( NodeType _type ) : base()
    {
      parent = 0;
      type = _type;
      firstChild = 0;
      lastChild = 0;
      prev = 0;
      next = 0;
    }


    node::~node()
    {
      node* xml_node = firstChild;
      node* temp = 0;

      while ( xml_node )
      {
        temp = xml_node;
        xml_node = xml_node->next;
        delete temp;
      }	
    }


    void node::CopyTo( node* target ) const
    {
      target->SetValue (value.c_str() );
      target->userData = userData; 
      target->location = location;
    }


    void node::Clear()
    {
      node* xml_node = firstChild;
      node* temp = 0;

      while ( xml_node )
      {
        temp = xml_node;
        xml_node = xml_node->next;
        delete temp;
      }	

      firstChild = 0;
      lastChild = 0;
    }


    node* node::LinkEndChild( node* xml_node )
    {
      assert( xml_node->parent == 0 || xml_node->parent == this );
      assert( xml_node->GetDocument() == 0 || xml_node->GetDocument() == this->GetDocument() );

      if ( xml_node->Type() == node::MGZXML_DOCUMENT )
      {
        delete xml_node;
        if ( GetDocument() ) 
          GetDocument()->SetError( MGZXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return 0;
      }

      xml_node->parent = this;

      xml_node->prev = lastChild;
      xml_node->next = 0;

      if ( lastChild )
        lastChild->next = xml_node;
      else
        firstChild = xml_node;			// it was an empty list.

      lastChild = xml_node;
      return xml_node;
    }


    node* node::InsertEndChild( const node& addThis )
    {
      if ( addThis.Type() == node::MGZXML_DOCUMENT )
      {
        if ( GetDocument() ) 
          GetDocument()->SetError( MGZXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return 0;
      }
      node* xml_node = addThis.Clone();
      if ( !xml_node )
        return 0;

      return LinkEndChild( xml_node );
    }


    node* node::InsertBeforeChild( node* beforeThis, const node& addThis )
    {	
      if ( !beforeThis || beforeThis->parent != this ) {
        return 0;
      }
      if ( addThis.Type() == node::MGZXML_DOCUMENT )
      {
        if ( GetDocument() ) 
          GetDocument()->SetError( MGZXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return 0;
      }

      node* xml_node = addThis.Clone();
      if ( !xml_node )
        return 0;
      xml_node->parent = this;

      xml_node->next = beforeThis;
      xml_node->prev = beforeThis->prev;
      if ( beforeThis->prev )
      {
        beforeThis->prev->next = xml_node;
      }
      else
      {
        assert( firstChild == beforeThis );
        firstChild = xml_node;
      }
      beforeThis->prev = xml_node;
      return xml_node;
    }


    node* node::InsertAfterChild( node* afterThis, const node& addThis )
    {
      if ( !afterThis || afterThis->parent != this ) {
        return 0;
      }
      if ( addThis.Type() == node::MGZXML_DOCUMENT )
      {
        if ( GetDocument() ) 
          GetDocument()->SetError( MGZXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return 0;
      }

      node* xml_node = addThis.Clone();
      if ( !xml_node )
        return 0;
      xml_node->parent = this;

      xml_node->prev = afterThis;
      xml_node->next = afterThis->next;
      if ( afterThis->next )
      {
        afterThis->next->prev = xml_node;
      }
      else
      {
        assert( lastChild == afterThis );
        lastChild = xml_node;
      }
      afterThis->next = xml_node;
      return xml_node;
    }


    node* node::ReplaceChild( node* replaceThis, const node& withThis )
    {
      if ( !replaceThis )
        return 0;

      if ( replaceThis->parent != this )
        return 0;

      if ( withThis.ToDocument() ) {
        // A document can never be a child.	Thanks to Noam.
        document* document = GetDocument();
        if ( document ) 
          document->SetError( MGZXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return 0;
      }

      node* xml_node = withThis.Clone();
      if ( !xml_node )
        return 0;

      xml_node->next = replaceThis->next;
      xml_node->prev = replaceThis->prev;

      if ( replaceThis->next )
        replaceThis->next->prev = xml_node;
      else
        lastChild = xml_node;

      if ( replaceThis->prev )
        replaceThis->prev->next = xml_node;
      else
        firstChild = xml_node;

      delete replaceThis;
      xml_node->parent = this;
      return xml_node;
    }


    bool node::RemoveChild( node* removeThis )
    {
      if ( !removeThis ) {
        return false;
      }

      if ( removeThis->parent != this )
      {	
        assert( 0 );
        return false;
      }

      if ( removeThis->next )
        removeThis->next->prev = removeThis->prev;
      else
        lastChild = removeThis->prev;

      if ( removeThis->prev )
        removeThis->prev->next = removeThis->next;
      else
        firstChild = removeThis->next;

      delete removeThis;
      return true;
    }

    const node* node::FirstChild( const char * _value ) const
    {
      const node* xml_node;
      for ( xml_node = firstChild; xml_node; xml_node = xml_node->next )
      {
        if ( strcmp( xml_node->Value(), _value ) == 0 )
          return xml_node;
      }
      return 0;
    }


    const node* node::LastChild( const char * _value ) const
    {
      const node* xml_node;
      for ( xml_node = lastChild; xml_node; xml_node = xml_node->prev )
      {
        if ( strcmp( xml_node->Value(), _value ) == 0 )
          return xml_node;
      }
      return 0;
    }


    const node* node::IterateChildren( const node* previous ) const
    {
      if ( !previous )
      {
        return FirstChild();
      }
      else
      {
        assert( previous->parent == this );
        return previous->NextSibling();
      }
    }


    const node* node::IterateChildren( const char * val, const node* previous ) const
    {
      if ( !previous )
      {
        return FirstChild( val );
      }
      else
      {
        assert( previous->parent == this );
        return previous->NextSibling( val );
      }
    }


    const node* node::NextSibling( const char * _value ) const 
    {
      const node* xml_node;
      for ( xml_node = next; xml_node; xml_node = xml_node->next )
      {
        if ( strcmp( xml_node->Value(), _value ) == 0 )
          return xml_node;
      }
      return 0;
    }


    const node* node::PreviousSibling( const char * _value ) const
    {
      const node* xml_node;
      for ( xml_node = prev; xml_node; xml_node = xml_node->prev )
      {
        if ( strcmp( xml_node->Value(), _value ) == 0 )
          return xml_node;
      }
      return 0;
    }


    void element::RemoveAttribute( const char * name )
    {
      MGZXML_STRING str( name );
      attribute* xml_node = xml_attributeSet.Find( str );
      if ( xml_node )
      {
        xml_attributeSet.Remove( xml_node );
        delete xml_node;
      }
    }

    const element* node::FirstChildElement() const
    {
      const node* xml_node;

      for (	xml_node = FirstChild();
          xml_node;
          xml_node = xml_node->NextSibling() )
      {
        if ( xml_node->ToElement() )
          return xml_node->ToElement();
      }
      return 0;
    }


    const element* node::FirstChildElement( const char * _value ) const
    {
      const node* xml_node;

      for (	xml_node = FirstChild( _value );
          xml_node;
          xml_node = xml_node->NextSibling( _value ) )
      {
        if ( xml_node->ToElement() )
          return xml_node->ToElement();
      }
      return 0;
    }


    const element* node::NextSiblingElement() const
    {
      const node* xml_node;

      for (	xml_node = NextSibling();
          xml_node;
          xml_node = xml_node->NextSibling() )
      {
        if ( xml_node->ToElement() )
          return xml_node->ToElement();
      }
      return 0;
    }


    const element* node::NextSiblingElement( const char * _value ) const
    {
      const node* xml_node;

      for (	xml_node = NextSibling( _value );
          xml_node;
          xml_node = xml_node->NextSibling( _value ) )
      {
        if ( xml_node->ToElement() )
          return xml_node->ToElement();
      }
      return 0;
    }


    const document* node::GetDocument() const
    {
      const node* xml_node;

      for( xml_node = this; xml_node; xml_node = xml_node->parent )
      {
        if ( xml_node->ToDocument() )
          return xml_node->ToDocument();
      }
      return 0;
    }


    element::element (const char * _value)
      : node( node::MGZXML_ELEMENT )
    {
      firstChild = lastChild = 0;
      value = _value;
    }


    element::element( const std::string& _value ) 
      : node( node::MGZXML_ELEMENT )
    {
      firstChild = lastChild = 0;
      value = _value;
    }


    element::element( const element& copy)
      : node( node::MGZXML_ELEMENT )
    {
      firstChild = lastChild = 0;
      copy.CopyTo( this );	
    }


    element& element::operator=( const element& base )
    {
      ClearThis();
      base.CopyTo( this );
      return *this;
    }


    element::~element()
    {
      ClearThis();
    }


    void element::ClearThis()
    {
      Clear();
      while( xml_attributeSet.First() )
      {
        attribute* xml_node = xml_attributeSet.First();
        xml_attributeSet.Remove( xml_node );
        delete xml_node;
      }
    }


    const char* element::Attribute( const char* name ) const
    {
      const attribute* xml_node = xml_attributeSet.Find( name );
      if ( xml_node )
        return xml_node->Value();
      return 0;
    }


    const std::string* element::Attribute( const std::string& name ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      if ( attrib )
        return &attrib->ValueStr();
      return 0;
    }


    const char* element::Attribute( const char* name, int* i ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      const char* result = 0;

      if ( attrib ) {
        result = attrib->Value();
        if ( i ) {
          attrib->QueryIntValue( i );
        }
      }
      return result;
    }


    const std::string* element::Attribute( const std::string& name, int* i ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      const std::string* result = 0;

      if ( attrib ) {
        result = &attrib->ValueStr();
        if ( i ) {
          attrib->QueryIntValue( i );
        }
      }
      return result;
    }


    const char* element::Attribute( const char* name, double* d ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      const char* result = 0;

      if ( attrib ) {
        result = attrib->Value();
        if ( d ) {
          attrib->QueryDoubleValue( d );
        }
      }
      return result;
    }


    const std::string* element::Attribute( const std::string& name, double* d ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      const std::string* result = 0;

      if ( attrib ) {
        result = &attrib->ValueStr();
        if ( d ) {
          attrib->QueryDoubleValue( d );
        }
      }
      return result;
    }


    int element::QueryIntAttribute( const char* name, int* ival ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      if ( !attrib )
        return MGZXML_NO_ATTRIBUTE;
      return attrib->QueryIntValue( ival );
    }


    int element::QueryUnsignedAttribute( const char* name, unsigned* value ) const
    {
      const attribute* xml_node = xml_attributeSet.Find( name );
      if ( !xml_node )
        return MGZXML_NO_ATTRIBUTE;

      int ival = 0;
      int result = xml_node->QueryIntValue( &ival );
      *value = (unsigned)ival;
      return result;
    }


    int element::QueryBoolAttribute( const char* name, bool* bval ) const
    {
      const attribute* xml_node = xml_attributeSet.Find( name );
      if ( !xml_node )
        return MGZXML_NO_ATTRIBUTE;

      int result = MGZXML_WRONG_TYPE;
      if (    StringEqual( xml_node->Value(), "true", true, MGZXML_ENCODING_UNKNOWN ) 
          || StringEqual( xml_node->Value(), "yes", true, MGZXML_ENCODING_UNKNOWN ) 
          || StringEqual( xml_node->Value(), "1", true, MGZXML_ENCODING_UNKNOWN ) ) 
      {
        *bval = true;
        result = MGZXML_SUCCESS;
      }
      else if (    StringEqual( xml_node->Value(), "false", true, MGZXML_ENCODING_UNKNOWN ) 
          || StringEqual( xml_node->Value(), "no", true, MGZXML_ENCODING_UNKNOWN ) 
          || StringEqual( xml_node->Value(), "0", true, MGZXML_ENCODING_UNKNOWN ) ) 
      {
        *bval = false;
        result = MGZXML_SUCCESS;
      }
      return result;
    }



    int element::QueryIntAttribute( const std::string& name, int* ival ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      if ( !attrib )
        return MGZXML_NO_ATTRIBUTE;
      return attrib->QueryIntValue( ival );
    }


    int element::QueryDoubleAttribute( const char* name, double* dval ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      if ( !attrib )
        return MGZXML_NO_ATTRIBUTE;
      return attrib->QueryDoubleValue( dval );
    }


    int element::QueryDoubleAttribute( const std::string& name, double* dval ) const
    {
      const attribute* attrib = xml_attributeSet.Find( name );
      if ( !attrib )
        return MGZXML_NO_ATTRIBUTE;
      return attrib->QueryDoubleValue( dval );
    }


    void element::SetAttribute( const char * name, int val )
    {	
      attribute* attrib = xml_attributeSet.FindOrCreate( name );
      if ( attrib ) {
        attrib->SetIntValue( val );
      }
    }


    void element::SetAttribute( const std::string& name, int val )
    {	
      attribute* attrib = xml_attributeSet.FindOrCreate( name );
      if ( attrib ) {
        attrib->SetIntValue( val );
      }
    }


    void element::SetDoubleAttribute( const char * name, double val )
    {	
      attribute* attrib = xml_attributeSet.FindOrCreate( name );
      if ( attrib ) {
        attrib->SetDoubleValue( val );
      }
    }


    void element::SetDoubleAttribute( const std::string& name, double val )
    {	
      attribute* attrib = xml_attributeSet.FindOrCreate( name );
      if ( attrib ) {
        attrib->SetDoubleValue( val );
      }
    }


    void element::SetAttribute( const char * cname, const char * cvalue )
    {
      attribute* attrib = xml_attributeSet.FindOrCreate( cname );
      if ( attrib ) {
        attrib->SetValue( cvalue );
      }
    }


    void element::SetAttribute( const std::string& _name, const std::string& _value )
    {
      attribute* attrib = xml_attributeSet.FindOrCreate( _name );
      if ( attrib ) {
        attrib->SetValue( _value );
      }
    }


    void element::Print( FILE* cfile, int depth ) const
    {
      int i;
      assert( cfile );
      for ( i=0; i<depth; i++ ) {
        fprintf( cfile, "    " );
      }

      fprintf( cfile, "<%s", value.c_str() );

      const attribute* attrib;
      for ( attrib = xml_attributeSet.First(); attrib; attrib = attrib->Next() )
      {
        fprintf( cfile, " " );
        attrib->Print( cfile, depth );
      }

      // There are 3 different formatting approaches:
      // 1) An element without children is printed as a <foo /> node
      // 2) An element with only a text child is printed as <foo> text </foo>
      // 3) An element with children is printed on multiple lines.
      node* xml_node;
      if ( !firstChild )
      {
        fprintf( cfile, " />" );
      }
      else if ( firstChild == lastChild && firstChild->ToText() )
      {
        fprintf( cfile, ">" );
        firstChild->Print( cfile, depth + 1 );
        fprintf( cfile, "</%s>", value.c_str() );
      }
      else
      {
        fprintf( cfile, ">" );

        for ( xml_node = firstChild; xml_node; xml_node=xml_node->NextSibling() )
        {
          if ( !xml_node->ToText() )
          {
            fprintf( cfile, "\n" );
          }
          xml_node->Print( cfile, depth+1 );
        }
        fprintf( cfile, "\n" );
        for( i=0; i<depth; ++i ) {
          fprintf( cfile, "    " );
        }
        fprintf( cfile, "</%s>", value.c_str() );
      }
    }


    void element::CopyTo( element* target ) const
    {
      // superclass:
      node::CopyTo( target );

      // Element class: 
      // Clone the attributes, then clone the children.
      const attribute* attribute = 0;
      for(	attribute = xml_attributeSet.First();
          attribute;
          attribute = attribute->Next() )
      {
        target->SetAttribute( attribute->Name(), attribute->Value() );
      }

      node* xml_node = 0;
      for ( xml_node = firstChild; xml_node; xml_node = xml_node->NextSibling() )
      {
        target->LinkEndChild( xml_node->Clone() );
      }
    }

    bool element::Accept( visitor* visitor ) const
    {
      if ( visitor->VisitEnter( *this, xml_attributeSet.First() ) ) 
      {
        for ( const node* xml_node=FirstChild(); xml_node; xml_node=xml_node->NextSibling() )
        {
          if ( !xml_node->Accept( visitor ) )
            break;
        }
      }
      return visitor->VisitExit( *this );
    }


    node* element::Clone() const
    {
      element* clone = new element( Value() );
      if ( !clone )
        return 0;

      CopyTo( clone );
      return clone;
    }


    const char* element::GetText() const
    {
      const node* child = this->FirstChild();
      if ( child ) {
        const text* childText = child->ToText();
        if ( childText ) {
          return childText->Value();
        }
      }
      return 0;
    }


    document::document() : node( node::MGZXML_DOCUMENT )
    {
      tabsize = 4;
      useMicrosoftBOM = false;
      ClearError();
    }

    document::document( const char * documentName ) : node( node::MGZXML_DOCUMENT )
    {
      tabsize = 4;
      useMicrosoftBOM = false;
      value = documentName;
      ClearError();
    }


    document::document( const std::string& documentName ) : node( node::MGZXML_DOCUMENT )
    {
      tabsize = 4;
      useMicrosoftBOM = false;
      value = documentName;
      ClearError();
    }


    document::document( const document& copy ) : node( node::MGZXML_DOCUMENT )
    {
      copy.CopyTo( this );
    }


    document& document::operator=( const document& copy )
    {
      Clear();
      copy.CopyTo( this );
      return *this;
    }


    bool document::LoadFile( encoding xml_encoding )
    {
      return LoadFile( Value(), xml_encoding );
    }


    bool document::SaveFile() const
    {
      return SaveFile( Value() );
    }

    bool document::LoadFile( const char* _filename, encoding xml_encoding )
    {
      MGZXML_STRING filename( _filename );
      value = filename;

      // reading in binary mode so that glowxml can normalize the EOL
      FILE* file = fOpen( value.c_str (), "rb" );	

      if ( file )
      {
        bool result = LoadFile( file, xml_encoding );
        fclose( file );
        return result;
      }
      else
      {
        SetError( MGZXML_ERROR_OPENING_FILE, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return false;
      }
    }

    bool document::LoadFile( FILE* file, encoding xml_encoding )
    {
      if ( !file ) 
      {
        SetError( MGZXML_ERROR_OPENING_FILE, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return false;
      }

      // Delete the existing data:
      Clear();
      location.Clear();

      // Get the file size, so we can pre-allocate the string. HUGE speed impact.
      long length = 0;
      fseek( file, 0, SEEK_END );
      length = ftell( file );
      fseek( file, 0, SEEK_SET );

      // Strange case, but good to handle up front.
      if ( length <= 0 )
      {
        SetError( MGZXML_ERROR_DOCUMENT_EMPTY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return false;
      }

      // Subtle bug here. mgz::xml did use fgets. But from the XML spec:
      // 2.11 End-of-Line Handling
      // <snip>
      // <quote>
      // ...the XML processor MUST behave as if it normalized all line breaks in external 
      // parsed entities (including the document entity) on input, before parsing, by translating 
      // both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
      // a single #xA character.
      // </quote>
      //
      // It is not clear fgets does that, and certainly isn't clear it works cross platform. 
      // Generally, you expect fgets to translate from the convention of the OS to the c/unix
      // convention, and not work generally.

      /*
         while( fgets( buf, sizeof(buf), file ) )
         {
         data += buf;
         }
         */

      char* buf = new char[ length+1 ];
      buf[0] = 0;

      if ( fread( buf, length, 1, file ) != 1 ) {
        delete [] buf;
        SetError( MGZXML_ERROR_OPENING_FILE, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return false;
      }

      // Process the buffer in place to normalize new lines. (See comment above.)
      // Copies from the 'p' to 'q' pointer, where p can advance faster if
      // a newline-carriage return is hit.
      //
      // Wikipedia:
      // Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or 
      // CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
      //		* LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
      //		* CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
      //		* CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9

      const char* p = buf;	// the read head
      char* q = buf;			// the write head
      const char CR = 0x0d;
      const char LF = 0x0a;

      buf[length] = 0;
      while( *p ) {
        assert( p < (buf+length) );
        assert( q <= (buf+length) );
        assert( q <= p );

        if ( *p == CR ) {
          *q++ = LF;
          p++;
          if ( *p == LF ) {		// check for CR+LF (and skip LF)
            p++;
          }
        }
        else {
          *q++ = *p++;
        }
      }
      assert( q <= (buf+length) );
      *q = 0;

      Parse( buf, 0, xml_encoding );

      delete [] buf;
      return !Error();
    }


    bool document::SaveFile( const char * filename ) const
    {
      // The old c stuff lives on...
      FILE* fp = fOpen( filename, "w" );
      if ( fp )
      {
        bool result = SaveFile( fp );
        fclose( fp );
        return result;
      }
      return false;
    }


    bool document::SaveFile( FILE* fp ) const
    {
      if ( useMicrosoftBOM ) 
      {
        const unsigned char MGZXML_UTF_LEAD_0 = 0xefU;
        const unsigned char MGZXML_UTF_LEAD_1 = 0xbbU;
        const unsigned char MGZXML_UTF_LEAD_2 = 0xbfU;

        fputc( MGZXML_UTF_LEAD_0, fp );
        fputc( MGZXML_UTF_LEAD_1, fp );
        fputc( MGZXML_UTF_LEAD_2, fp );
      }
      Print( fp, 0 );
      return (ferror(fp) == 0);
    }


    void document::CopyTo( document* target ) const
    {
      node::CopyTo( target );

      target->error = error;
      target->errorId = errorId;
      target->errorDesc = errorDesc;
      target->tabsize = tabsize;
      target->errorLocation = errorLocation;
      target->useMicrosoftBOM = useMicrosoftBOM;

      node* xml_node = 0;
      for ( xml_node = firstChild; xml_node; xml_node = xml_node->NextSibling() )
      {
        target->LinkEndChild( xml_node->Clone() );
      }	
    }


    node* document::Clone() const
    {
      document* clone = new document();
      if ( !clone )
        return 0;

      CopyTo( clone );
      return clone;
    }


    void document::Print( FILE* cfile, int depth ) const
    {
      assert( cfile );
      for ( const node* xml_node=FirstChild(); xml_node; xml_node=xml_node->NextSibling() )
      {
        xml_node->Print( cfile, depth );
        fprintf( cfile, "\n" );
      }
    }


    bool document::Accept( visitor* visitor ) const
    {
      if ( visitor->VisitEnter( *this ) )
      {
        for ( const node* xml_node=FirstChild(); xml_node; xml_node=xml_node->NextSibling() )
        {
          if ( !xml_node->Accept( visitor ) )
            break;
        }
      }
      return visitor->VisitExit( *this );
    }


    const attribute* attribute::Next() const
    {
      // We are using knowledge of the sentinel. The sentinel
      // have a value or name.
      if ( next->value.empty() && next->name.empty() )
        return 0;
      return next;
    }

    /*
       attribute* attribute::Next()
       {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if ( next->value.empty() && next->name.empty() )
    return 0;
    return next;
    }
    */

    const attribute* attribute::Previous() const
    {
      // We are using knowledge of the sentinel. The sentinel
      // have a value or name.
      if ( prev->value.empty() && prev->name.empty() )
        return 0;
      return prev;
    }

    /*
       attribute* attribute::Previous()
       {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if ( prev->value.empty() && prev->name.empty() )
    return 0;
    return prev;
    }
    */

    void attribute::Print( FILE* cfile, int /*depth*/, MGZXML_STRING* str ) const
    {
      MGZXML_STRING n, v;

      EncodeString( name, &n );
      EncodeString( value, &v );

      if (value.find ('\"') == MGZXML_STRING::npos) {
        if ( cfile ) {
          fprintf (cfile, "%s=\"%s\"", n.c_str(), v.c_str() );
        }
        if ( str ) {
          (*str) += n; (*str) += "=\""; (*str) += v; (*str) += "\"";
        }
      }
      else {
        if ( cfile ) {
          fprintf (cfile, "%s='%s'", n.c_str(), v.c_str() );
        }
        if ( str ) {
          (*str) += n; (*str) += "='"; (*str) += v; (*str) += "'";
        }
      }
    }


    int attribute::QueryIntValue( int* ival ) const
    {
      if ( MGZXML_SSCANF( value.c_str(), "%d", ival ) == 1 )
        return MGZXML_SUCCESS;
      return MGZXML_WRONG_TYPE;
    }

    int attribute::QueryDoubleValue( double* dval ) const
    {
      if ( MGZXML_SSCANF( value.c_str(), "%lf", dval ) == 1 )
        return MGZXML_SUCCESS;
      return MGZXML_WRONG_TYPE;
    }

    void attribute::SetIntValue( int _value )
    {
      char buf [64];
#if defined(MGZXML_SNPRINTF)		
      MGZXML_SNPRINTF(buf, sizeof(buf), "%d", _value);
#else
      sprintf (buf, "%d", _value);
#endif
      SetValue (buf);
    }

    void attribute::SetDoubleValue( double _value )
    {
      char buf [256];
#if defined(MGZXML_SNPRINTF)		
      MGZXML_SNPRINTF( buf, sizeof(buf), "%g", _value);
#else
      sprintf (buf, "%g", _value);
#endif
      SetValue (buf);
    }

    int attribute::IntValue() const
    {
      return atoi (value.c_str ());
    }

    double  attribute::DoubleValue() const
    {
      return atof (value.c_str ());
    }


    comment::comment( const comment& copy ) : node( node::MGZXML_COMMENT )
    {
      copy.CopyTo( this );
    }


    comment& comment::operator=( const comment& base )
    {
      Clear();
      base.CopyTo( this );
      return *this;
    }


    void comment::Print( FILE* cfile, int depth ) const
    {
      assert( cfile );
      for ( int i=0; i<depth; i++ )
      {
        fprintf( cfile,  "    " );
      }
      fprintf( cfile, "<!--%s-->", value.c_str() );
    }


    void comment::CopyTo( comment* target ) const
    {
      node::CopyTo( target );
    }


    bool comment::Accept( visitor* visitor ) const
    {
      return visitor->Visit( *this );
    }


    node* comment::Clone() const
    {
      comment* clone = new comment();

      if ( !clone )
        return 0;

      CopyTo( clone );
      return clone;
    }


    void text::Print( FILE* cfile, int depth ) const
    {
      assert( cfile );
      if ( cdata )
      {
        int i;
        fprintf( cfile, "\n" );
        for ( i=0; i<depth; i++ ) {
          fprintf( cfile, "    " );
        }
        fprintf( cfile, "<![CDATA[%s]]>\n", value.c_str() );	// unformatted output
      }
      else
      {
        MGZXML_STRING buffer;
        EncodeString( value, &buffer );
        fprintf( cfile, "%s", buffer.c_str() );
      }
    }


    void text::CopyTo( text* target ) const
    {
      node::CopyTo( target );
      target->cdata = cdata;
    }


    bool text::Accept( visitor* visitor ) const
    {
      return visitor->Visit( *this );
    }


    node* text::Clone() const
    {	
      text* clone = 0;
      clone = new text( "" );

      if ( !clone )
        return 0;

      CopyTo( clone );
      return clone;
    }


    declaration::declaration( const char * _version,
        const char * _xml_encoding,
        const char * _standalone )
      : node( node::MGZXML_DECLARATION )
    {
      version = _version;
      xml_encoding = _xml_encoding;
      standalone = _standalone;
    }


    declaration::declaration(	const std::string& _version,
        const std::string& _xml_encoding,
        const std::string& _standalone )
      : node( node::MGZXML_DECLARATION )
    {
      version = _version;
      xml_encoding = _xml_encoding;
      standalone = _standalone;
    }


    declaration::declaration( const declaration& copy )
      : node( node::MGZXML_DECLARATION )
    {
      copy.CopyTo( this );	
    }


    declaration& declaration::operator=( const declaration& copy )
    {
      Clear();
      copy.CopyTo( this );
      return *this;
    }


    void declaration::Print( FILE* cfile, int /*depth*/, MGZXML_STRING* str ) const
    {
      if ( cfile ) fprintf( cfile, "<?xml " );
      if ( str )	 (*str) += "<?xml ";

      if ( !version.empty() ) {
        if ( cfile ) fprintf (cfile, "version=\"%s\" ", version.c_str ());
        if ( str ) { (*str) += "version=\""; (*str) += version; (*str) += "\" "; }
      }
      if ( !xml_encoding.empty() ) {
        if ( cfile ) fprintf (cfile, "encoding=\"%s\" ", xml_encoding.c_str ());
        if ( str ) { (*str) += "encoding=\""; (*str) += xml_encoding; (*str) += "\" "; }
      }
      if ( !standalone.empty() ) {
        if ( cfile ) fprintf (cfile, "standalone=\"%s\" ", standalone.c_str ());
        if ( str ) { (*str) += "standalone=\""; (*str) += standalone; (*str) += "\" "; }
      }
      if ( cfile ) fprintf( cfile, "?>" );
      if ( str )	 (*str) += "?>";
    }


    void declaration::CopyTo( declaration* target ) const
    {
      node::CopyTo( target );

      target->version = version;
      target->xml_encoding = xml_encoding;
      target->standalone = standalone;
    }


    bool declaration::Accept( visitor* visitor ) const
    {
      return visitor->Visit( *this );
    }


    node* declaration::Clone() const
    {	
      declaration* clone = new declaration();

      if ( !clone )
        return 0;

      CopyTo( clone );
      return clone;
    }


    void unknown::Print( FILE* cfile, int depth ) const
    {
      for ( int i=0; i<depth; i++ )
        fprintf( cfile, "    " );
      fprintf( cfile, "<%s>", value.c_str() );
    }


    void unknown::CopyTo( unknown* target ) const
    {
      node::CopyTo( target );
    }


    bool unknown::Accept( visitor* visitor ) const
    {
      return visitor->Visit( *this );
    }


    node* unknown::Clone() const
    {
      unknown* clone = new unknown();

      if ( !clone )
        return 0;

      CopyTo( clone );
      return clone;
    }


    attributeSet::attributeSet()
    {
      sentinel.next = &sentinel;
      sentinel.prev = &sentinel;
    }


    attributeSet::~attributeSet()
    {
      assert( sentinel.next == &sentinel );
      assert( sentinel.prev == &sentinel );
    }


    void attributeSet::Add( attribute* addMe )
    {
      assert( !Find( MGZXML_STRING( addMe->Name() ) ) );	// Shouldn't be multiply adding to the set.

      addMe->next = &sentinel;
      addMe->prev = sentinel.prev;

      sentinel.prev->next = addMe;
      sentinel.prev      = addMe;
    }

    void attributeSet::Remove( attribute* removeMe )
    {
      attribute* xml_node;

      for( xml_node = sentinel.next; xml_node != &sentinel; xml_node = xml_node->next )
      {
        if ( xml_node == removeMe )
        {
          xml_node->prev->next = xml_node->next;
          xml_node->next->prev = xml_node->prev;
          xml_node->next = 0;
          xml_node->prev = 0;
          return;
        }
      }
      assert( 0 );		// we tried to remove a non-linked attribute.
    }


    attribute* attributeSet::Find( const std::string& name ) const
    {
      for( attribute* xml_node = sentinel.next; xml_node != &sentinel; xml_node = xml_node->next )
      {
        if ( xml_node->name == name )
          return xml_node;
      }
      return 0;
    }

    attribute* attributeSet::FindOrCreate( const std::string& _name )
    {
      attribute* attrib = Find( _name );
      if ( !attrib ) {
        attrib = new attribute();
        Add( attrib );
        attrib->SetName( _name );
      }
      return attrib;
    }


    attribute* attributeSet::Find( const char* name ) const
    {
      for( attribute* xml_node = sentinel.next; xml_node != &sentinel; xml_node = xml_node->next )
      {
        if ( strcmp( xml_node->name.c_str(), name ) == 0 )
          return xml_node;
      }
      return 0;
    }


    attribute* attributeSet::FindOrCreate( const char* _name )
    {
      attribute* attrib = Find( _name );
      if ( !attrib ) {
        attrib = new attribute();
        Add( attrib );
        attrib->SetName( _name );
      }
      return attrib;
    }


    std::istream& operator>> (std::istream & in, node & base)
    {
      MGZXML_STRING tag;
      tag.reserve( 8 * 1000 );
      base.StreamIn( &in, &tag );

      base.Parse( tag.c_str(), 0, MGZXML_DEFAULT_ENCODING );
      return in;
    }


    std::ostream& operator<< (std::ostream & out, const node & base)
    {
      printer printer;
      printer.SetStreamPrinting();
      base.Accept( &printer );
      out << printer.Str();

      return out;
    }


    std::string& operator<< (std::string& out, const node& base )
    {
      printer printer;
      printer.SetStreamPrinting();
      base.Accept( &printer );
      out.append( printer.Str() );

      return out;
    }


    handle handle::FirstChild() const
    {
      if ( xml_node )
      {
        node* child = xml_node->FirstChild();
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    handle handle::FirstChild( const char * value ) const
    {
      if ( xml_node )
      {
        node* child = xml_node->FirstChild( value );
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    handle handle::FirstChildElement() const
    {
      if ( xml_node )
      {
        element* child = xml_node->FirstChildElement();
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    handle handle::FirstChildElement( const char * value ) const
    {
      if ( xml_node )
      {
        element* child = xml_node->FirstChildElement( value );
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    handle handle::Child( int count ) const
    {
      if ( xml_node )
      {
        int i;
        node* child = xml_node->FirstChild();
        for (	i=0;
            child && i<count;
            child = child->NextSibling(), ++i )
        {
          // nothing
        }
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    handle handle::Child( const char* value, int count ) const
    {
      if ( xml_node )
      {
        int i;
        node* child = xml_node->FirstChild( value );
        for (	i=0;
            child && i<count;
            child = child->NextSibling( value ), ++i )
        {
          // nothing
        }
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    handle handle::ChildElement( int count ) const
    {
      if ( xml_node )
      {
        int i;
        element* child = xml_node->FirstChildElement();
        for (	i=0;
            child && i<count;
            child = child->NextSiblingElement(), ++i )
        {
          // nothing
        }
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    handle handle::ChildElement( const char* value, int count ) const
    {
      if ( xml_node )
      {
        int i;
        element* child = xml_node->FirstChildElement( value );
        for (	i=0;
            child && i<count;
            child = child->NextSiblingElement( value ), ++i )
        {
          // nothing
        }
        if ( child )
          return handle( child );
      }
      return handle( 0 );
    }


    bool printer::VisitEnter( const document& )
    {
      return true;
    }

    bool printer::VisitExit( const document& )
    {
      return true;
    }

    bool printer::VisitEnter( const element& element, const attribute* firstAttribute )
    {
      DoIndent();
      buffer += "<";
      buffer += element.Value();

      for( const attribute* attrib = firstAttribute; attrib; attrib = attrib->Next() )
      {
        buffer += " ";
        attrib->Print( 0, 0, &buffer );
      }

      if ( !element.FirstChild() ) 
      {
        buffer += " />";
        DoLineBreak();
      }
      else 
      {
        buffer += ">";
        if (    element.FirstChild()->ToText()
            && element.LastChild() == element.FirstChild()
            && element.FirstChild()->ToText()->CDATA() == false )
        {
          simpleTextPrint = true;
          // no DoLineBreak()!
        }
        else
        {
          DoLineBreak();
        }
      }
      ++depth;	
      return true;
    }


    bool printer::VisitExit( const element& element )
    {
      --depth;
      if ( !element.FirstChild() ) 
      {
        // nothing.
      }
      else 
      {
        if ( simpleTextPrint )
        {
          simpleTextPrint = false;
        }
        else
        {
          DoIndent();
        }
        buffer += "</";
        buffer += element.Value();
        buffer += ">";
        DoLineBreak();
      }
      return true;
    }


    bool printer::Visit( const text& text )
    {
      if ( text.CDATA() )
      {
        DoIndent();
        buffer += "<![CDATA[";
        buffer += text.Value();
        buffer += "]]>";
        DoLineBreak();
      }
      else if ( simpleTextPrint )
      {
        MGZXML_STRING str;
        base::EncodeString( text.ValueTStr(), &str );
        buffer += str;
      }
      else
      {
        DoIndent();
        MGZXML_STRING str;
        base::EncodeString( text.ValueTStr(), &str );
        buffer += str;
        DoLineBreak();
      }
      return true;
    }


    bool printer::Visit( const declaration& declaration )
    {
      DoIndent();
      declaration.Print( 0, 0, &buffer );
      DoLineBreak();
      return true;
    }


    bool printer::Visit( const comment& comment )
    {
      DoIndent();
      buffer += "<!--";
      buffer += comment.Value();
      buffer += "-->";
      DoLineBreak();
      return true;
    }


    bool printer::Visit( const unknown& unknown )
    {
      DoIndent();
      buffer += "<";
      buffer += unknown.Value();
      buffer += ">";
      DoLineBreak();
      return true;
    }
  }
}
