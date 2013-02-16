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
#include <stddef.h>

#include "xml/xml.h"

//#define DEBUG_PARSER
#if defined( DEBUG_PARSER )
#	if defined( DEBUG ) && defined( _MSC_VER )
#		include <windows.h>
#		define MGZXML_LOG OutputDebugString
#	else
#		define MGZXML_LOG printf
#	endif
#endif

namespace mgz {
  namespace xml {
    // Note tha "PutString" hardcodes the same list. This
    // is less flexible than it appears. Changing the entries
    // or order will break putstring.	
    base::Entity base::entity[ base::NUM_ENTITY ] = 
    {
      { "&amp;",  5, '&' },
      { "&lt;",   4, '<' },
      { "&gt;",   4, '>' },
      { "&quot;", 6, '\"' },
      { "&apos;", 6, '\'' }
    };

    // Bunch of unicode info at:
    //		http://www.unicode.org/faq/utf_bom.html
    // Including the basic of this table, which determines the #bytes in the
    // sequence from the lead byte. 1 placed for invalid sequences --
    // although the result will be junk, pass it through as much as possible.
    // Beware of the non-characters in UTF-8:	
    //				ef bb bf (Microsoft "lead bytes")
    //				ef bf be
    //				ef bf bf 

    const unsigned char MGZXML_UTF_LEAD_0 = 0xefU;
    const unsigned char MGZXML_UTF_LEAD_1 = 0xbbU;
    const unsigned char MGZXML_UTF_LEAD_2 = 0xbfU;

    const int base::utf8ByteTable[256] = 
    {
      //	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x00
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x10
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x20
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x30
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x40
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x50
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x60
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x70	End of ASCII range
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x80 0x80 to 0xc1 invalid
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x90 
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xa0 
      1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xb0 
      1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xc0 0xc2 to 0xdf 2 byte
      2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xd0
      3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	// 0xe0 0xe0 to 0xef 3 byte
      4,	4,	4,	4,	4,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1	// 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
    };


    void base::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
    {
      const unsigned long BYTE_MASK = 0xBF;
      const unsigned long BYTE_MARK = 0x80;
      const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

      if (input < 0x80) 
        *length = 1;
      else if ( input < 0x800 )
        *length = 2;
      else if ( input < 0x10000 )
        *length = 3;
      else if ( input < 0x200000 )
        *length = 4;
      else
      { *length = 0; return; }	// This code won't covert this correctly anyway.

      output += *length;

      // Scary scary fall throughs.
      switch (*length) 
      {
        case 4:
          --output; 
          *output = (char)((input | BYTE_MARK) & BYTE_MASK); 
          input >>= 6;
        case 3:
          --output; 
          *output = (char)((input | BYTE_MARK) & BYTE_MASK); 
          input >>= 6;
        case 2:
          --output; 
          *output = (char)((input | BYTE_MARK) & BYTE_MASK); 
          input >>= 6;
        case 1:
          --output; 
          *output = (char)(input | FIRST_BYTE_MARK[*length]);
      }
    }


    /*static*/ int base::IsAlpha( unsigned char anyByte, encoding /*xml_encoding*/ )
    {
      // This will only work for low-ascii, everything else is assumed to be a valid
      // letter. I'm not sure this is the best approach, but it is quite tricky trying
      // to figure out alhabetical vs. not across encoding. So take a very 
      // conservative approach.

      //	if ( xml_encoding == MGZXML_ENCODING_UTF8 )
      //	{
      if ( anyByte < 127 )
        return isalpha( anyByte );
      else
        return 1;	// What else to do? The unicode set is huge...get the english ones right.
      //	}
      //	else
      //	{
      //		return isalpha( anyByte );
      //	}
    }


    /*static*/ int base::IsAlphaNum( unsigned char anyByte, encoding /*xml_encoding*/ )
    {
      // This will only work for low-ascii, everything else is assumed to be a valid
      // letter. I'm not sure this is the best approach, but it is quite tricky trying
      // to figure out alhabetical vs. not across encoding. So take a very 
      // conservative approach.

      //	if ( xml_encoding == MGZXML_ENCODING_UTF8 )
      //	{
      if ( anyByte < 127 )
        return isalnum( anyByte );
      else
        return 1;	// What else to do? The unicode set is huge...get the english ones right.
      //	}
      //	else
      //	{
      //		return isalnum( anyByte );
      //	}
    }


    class parsingData
    {
      friend class document;
      public:
      void Stamp( const char* now, encoding xml_encoding );

      const cursor& Cursor() const	{ return xml_cursor; }

      private:
      // Only used by the document!
      parsingData( const char* start, int _tabsize, int row, int col )
      {
        assert( start );
        stamp = start;
        tabsize = _tabsize;
        xml_cursor.row = row;
        xml_cursor.col = col;
      }

      cursor		xml_cursor;
      const char*		stamp;
      int				tabsize;
    };


    void parsingData::Stamp( const char* now, encoding xml_encoding )
    {
      assert( now );

      // Do nothing if the tabsize is 0.
      if ( tabsize < 1 )
      {
        return;
      }

      // Get the current row, column.
      int row = xml_cursor.row;
      int col = xml_cursor.col;
      const char* p = stamp;
      assert( p );

      while ( p < now )
      {
        // Treat p as unsigned, so we have a happy compiler.
        const unsigned char* pU = (const unsigned char*)p;

        // Code contributed by Fletcher Dunn: (modified by lee)
        switch (*pU) {
          case 0:
            // We *should* never get here, but in case we do, don't
            // advance past the terminating null character, ever
            return;

          case '\r':
            // bump down to the next line
            ++row;
            col = 0;				
            // Eat the character
            ++p;

            // Check for \r\n sequence, and treat this as a single character
            if (*p == '\n') {
              ++p;
            }
            break;

          case '\n':
            // bump down to the next line
            ++row;
            col = 0;

            // Eat the character
            ++p;

            // Check for \n\r sequence, and treat this as a single
            // character.  (Yes, this bizarre thing does occur still
            // on some arcane platforms...)
            if (*p == '\r') {
              ++p;
            }
            break;

          case '\t':
            // Eat the character
            ++p;

            // Skip to next tab stop
            col = (col / tabsize + 1) * tabsize;
            break;

          case MGZXML_UTF_LEAD_0:
            if ( xml_encoding == MGZXML_ENCODING_UTF8 )
            {
              if ( *(p+1) && *(p+2) )
              {
                // In these cases, don't advance the column. These are
                // 0-width spaces.
                if ( *(pU+1)==MGZXML_UTF_LEAD_1 && *(pU+2)==MGZXML_UTF_LEAD_2 )
                  p += 3;	
                else if ( *(pU+1)==0xbfU && *(pU+2)==0xbeU )
                  p += 3;	
                else if ( *(pU+1)==0xbfU && *(pU+2)==0xbfU )
                  p += 3;	
                else
                { p +=3; ++col; }	// A normal character.
              }
            }
            else
            {
              ++p;
              ++col;
            }
            break;

          default:
            if ( xml_encoding == MGZXML_ENCODING_UTF8 )
            {
              // Eat the 1 to 4 byte utf8 character.
              int step = base::utf8ByteTable[*((const unsigned char*)p)];
              if ( step == 0 )
                step = 1;		// Error case from bad encoding, but handle gracefully.
              p += step;

              // Just advance one column, of course.
              ++col;
            }
            else
            {
              ++p;
              ++col;
            }
            break;
        }
      }
      xml_cursor.row = row;
      xml_cursor.col = col;
      assert( xml_cursor.row >= -1 );
      assert( xml_cursor.col >= -1 );
      stamp = p;
      assert( stamp );
    }


    const char* base::SkipWhiteSpace( const char* p, encoding xml_encoding )
    {
      if ( !p || !*p )
      {
        return 0;
      }
      if ( xml_encoding == MGZXML_ENCODING_UTF8 )
      {
        while ( *p )
        {
          const unsigned char* pU = (const unsigned char*)p;

          // Skip the stupid Microsoft UTF-8 Byte order marks
          if (	*(pU+0)==MGZXML_UTF_LEAD_0
              && *(pU+1)==MGZXML_UTF_LEAD_1 
              && *(pU+2)==MGZXML_UTF_LEAD_2 )
          {
            p += 3;
            continue;
          }
          else if(*(pU+0)==MGZXML_UTF_LEAD_0
              && *(pU+1)==0xbfU
              && *(pU+2)==0xbeU )
          {
            p += 3;
            continue;
          }
          else if(*(pU+0)==MGZXML_UTF_LEAD_0
              && *(pU+1)==0xbfU
              && *(pU+2)==0xbfU )
          {
            p += 3;
            continue;
          }

          if ( IsWhiteSpace( *p ) )		// Still using old rules for white space.
            ++p;
          else
            break;
        }
      }
      else
      {
        while ( *p && IsWhiteSpace( *p ) )
          ++p;
      }

      return p;
    }

    /*static*/ bool base::StreamWhiteSpace( std::istream * in, MGZXML_STRING * tag )
    {
      for( ;; )
      {
        if ( !in->good() ) return false;

        int c = in->peek();
        // At this scope, we can't get to a document. So fail silently.
        if ( !IsWhiteSpace( c ) || c <= 0 )
          return true;

        *tag += (char) in->get();
      }
    }

    /*static*/ bool base::StreamTo( std::istream * in, int character, MGZXML_STRING * tag )
    {
      //assert( character > 0 && character < 128 );	// else it won't work in utf-8
      while ( in->good() )
      {
        int c = in->peek();
        if ( c == character )
          return true;
        if ( c <= 0 )		// Silent failure: can't get document at this scope
          return false;

        in->get();
        *tag += (char) c;
      }
      return false;
    }

    // One of mgz::xml's more performance demanding functions. Try to keep the memory overhead down. The
    // "assign" optimization removes over 10% of the execution time.
    //
    const char* base::ReadName( const char* p, MGZXML_STRING * name, encoding xml_encoding )
    {
      // Oddly, not supported on some comilers,
      //name->clear();
      // So use this:
      *name = "";
      assert( p );

      // Names start with letters or underscores.
      // Of course, in unicode, glowxml has no idea what a letter *is*. The
      // algorithm is generous.
      //
      // After that, they can be letters, underscores, numbers,
      // hyphens, or colons. (Colons are valid ony for namespaces,
      // but glowxml can't tell namespaces from names.)
      if (    p && *p 
          && ( IsAlpha( (unsigned char) *p, xml_encoding ) || *p == '_' ) )
      {
        const char* start = p;
        while(		p && *p
            &&	(		IsAlphaNum( (unsigned char ) *p, xml_encoding ) 
              || *p == '_'
              || *p == '-'
              || *p == '.'
              || *p == ':' ) )
        {
          //(*name) += *p; // expensive
          ++p;
        }
        if ( p-start > 0 ) {
          name->assign( start, p-start );
        }
        return p;
      }
      return 0;
    }

    const char* base::GetEntity( const char* p, char* value, int* length, encoding xml_encoding )
    {
      // Presume an entity, and pull it out.
      MGZXML_STRING ent;
      int i;
      *length = 0;

      if ( *(p+1) && *(p+1) == '#' && *(p+2) )
      {
        unsigned long ucs = 0;
        ptrdiff_t delta = 0;
        unsigned mult = 1;

        if ( *(p+2) == 'x' )
        {
          // Hexadecimal.
          if ( !*(p+3) ) return 0;

          const char* q = p+3;
          q = strchr( q, ';' );

          if ( !q || !*q ) return 0;

          delta = q-p;
          --q;

          while ( *q != 'x' )
          {
            if ( *q >= '0' && *q <= '9' )
              ucs += mult * (*q - '0');
            else if ( *q >= 'a' && *q <= 'f' )
              ucs += mult * (*q - 'a' + 10);
            else if ( *q >= 'A' && *q <= 'F' )
              ucs += mult * (*q - 'A' + 10 );
            else 
              return 0;
            mult *= 16;
            --q;
          }
        }
        else
        {
          // Decimal.
          if ( !*(p+2) ) return 0;

          const char* q = p+2;
          q = strchr( q, ';' );

          if ( !q || !*q ) return 0;

          delta = q-p;
          --q;

          while ( *q != '#' )
          {
            if ( *q >= '0' && *q <= '9' )
              ucs += mult * (*q - '0');
            else 
              return 0;
            mult *= 10;
            --q;
          }
        }
        if ( xml_encoding == MGZXML_ENCODING_UTF8 )
        {
          // convert the UCS to UTF-8
          ConvertUTF32ToUTF8( ucs, value, length );
        }
        else
        {
          *value = (char)ucs;
          *length = 1;
        }
        return p + delta + 1;
      }

      // Now try to match it.
      for( i=0; i<NUM_ENTITY; ++i )
      {
        if ( strncmp( entity[i].str, p, entity[i].strLength ) == 0 )
        {
          assert( strlen( entity[i].str ) == entity[i].strLength );
          *value = entity[i].chr;
          *length = 1;
          return ( p + entity[i].strLength );
        }
      }

      // So it wasn't an entity, its unrecognized, or something like that.
      *value = *p;	// Don't put back the last one, since we return it!
      //*length = 1;	// Leave unrecognized entities - this doesn't really work.
      // Just writes strange XML.
      return p+1;
    }


    bool base::StringEqual( const char* p,
        const char* tag,
        bool ignoreCase,
        encoding xml_encoding )
    {
      assert( p );
      assert( tag );
      if ( !p || !*p )
      {
        assert( 0 );
        return false;
      }

      const char* q = p;

      if ( ignoreCase )
      {
        while ( *q && *tag && ToLower( *q, xml_encoding ) == ToLower( *tag, xml_encoding ) )
        {
          ++q;
          ++tag;
        }

        if ( *tag == 0 )
          return true;
      }
      else
      {
        while ( *q && *tag && *q == *tag )
        {
          ++q;
          ++tag;
        }

        if ( *tag == 0 )		// Have we found the end of the tag, and everything equal?
          return true;
      }
      return false;
    }

    const char* base::ReadText(	const char* p, 
        MGZXML_STRING * xml_text, 
        bool trimWhiteSpace, 
        const char* endTag, 
        bool caseInsensitive,
        encoding xml_encoding )
    {
      *xml_text = "";
      if (    !trimWhiteSpace			// certain tags always keep whitespace
          || !condenseWhiteSpace )	// if true, whitespace is always kept
      {
        // Keep all the white space.
        while (	   p && *p
            && !StringEqual( p, endTag, caseInsensitive, xml_encoding )
            )
        {
          int len;
          char cArr[4] = { 0, 0, 0, 0 };
          p = GetChar( p, cArr, &len, xml_encoding );
          xml_text->append( cArr, len );
        }
      }
      else
      {
        bool whitespace = false;

        // Remove leading white space:
        p = SkipWhiteSpace( p, xml_encoding );
        while (	   p && *p
            && !StringEqual( p, endTag, caseInsensitive, xml_encoding ) )
        {
          if ( *p == '\r' || *p == '\n' )
          {
            whitespace = true;
            ++p;
          }
          else if ( IsWhiteSpace( *p ) )
          {
            whitespace = true;
            ++p;
          }
          else
          {
            // If we've found whitespace, add it before the
            // new character. Any whitespace just becomes a space.
            if ( whitespace )
            {
              (*xml_text) += ' ';
              whitespace = false;
            }
            int len;
            char cArr[4] = { 0, 0, 0, 0 };
            p = GetChar( p, cArr, &len, xml_encoding );
            if ( len == 1 )
              (*xml_text) += cArr[0];	// more efficient
            else
              xml_text->append( cArr, len );
          }
        }
      }
      if ( p && *p )
        p += strlen( endTag );
      return ( p && *p ) ? p : 0;
    }

    void document::StreamIn( std::istream * in, MGZXML_STRING * tag )
    {
      // The basic issue with a document is that we don't know what we're
      // streaming. Read something presumed to be a tag (and hope), then
      // identify it, and call the appropriate stream method on the tag.
      //
      // This "pre-streaming" will never read the closing ">" so the
      // sub-tag can orient itself.

      if ( !StreamTo( in, '<', tag ) ) 
      {
        SetError( MGZXML_ERROR_PARSING_EMPTY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return;
      }

      while ( in->good() )
      {
        int tagIndex = (int) tag->length();
        while ( in->good() && in->peek() != '>' )
        {
          int c = in->get();
          if ( c <= 0 )
          {
            SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
            break;
          }
          (*tag) += (char) c;
        }

        if ( in->good() )
        {
          // We now have something we presume to be a node of 
          // some sort. Identify it, and call the node to
          // continue streaming.
          node* xml_node = Identify( tag->c_str() + tagIndex, MGZXML_DEFAULT_ENCODING );

          if ( xml_node )
          {
            xml_node->StreamIn( in, tag );
            bool isElement = xml_node->ToElement() != 0;
            delete xml_node;
            xml_node = 0;

            // If this is the root element, we're done. Parsing will be
            // done by the >> operator.
            if ( isElement )
            {
              return;
            }
          }
          else
          {
            SetError( MGZXML_ERROR, 0, 0, MGZXML_ENCODING_UNKNOWN );
            return;
          }
        }
      }
      // We should have returned sooner.
      SetError( MGZXML_ERROR, 0, 0, MGZXML_ENCODING_UNKNOWN );
    }

    const char* document::Parse( const char* p, parsingData* prevData, encoding xml_encoding )
    {
      ClearError();

      // Parse away, at the document level. Since a document
      // contains nothing but other tags, most of what happens
      // here is skipping white space.
      if ( !p || !*p )
      {
        SetError( MGZXML_ERROR_DOCUMENT_EMPTY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return 0;
      }

      // Note that, for a document, this needs to come
      // before the while space skip, so that parsing
      // starts from the pointer we are given.
      location.Clear();
      if ( prevData )
      {
        location.row = prevData->xml_cursor.row;
        location.col = prevData->xml_cursor.col;
      }
      else
      {
        location.row = 0;
        location.col = 0;
      }
      parsingData data( p, TabSize(), location.row, location.col );
      location = data.Cursor();

      if ( xml_encoding == MGZXML_ENCODING_UNKNOWN )
      {
        // Check for the Microsoft UTF-8 lead bytes.
        const unsigned char* pU = (const unsigned char*)p;
        if (	*(pU+0) && *(pU+0) == MGZXML_UTF_LEAD_0
            && *(pU+1) && *(pU+1) == MGZXML_UTF_LEAD_1
            && *(pU+2) && *(pU+2) == MGZXML_UTF_LEAD_2 )
        {
          xml_encoding = MGZXML_ENCODING_UTF8;
          useMicrosoftBOM = true;
        }
      }

      p = SkipWhiteSpace( p, xml_encoding );
      if ( !p )
      {
        SetError( MGZXML_ERROR_DOCUMENT_EMPTY, 0, 0, MGZXML_ENCODING_UNKNOWN );
        return 0;
      }

      while ( p && *p )
      {
        node* xml_node = Identify( p, xml_encoding );
        if ( xml_node )
        {
          p = xml_node->Parse( p, &data, xml_encoding );
          LinkEndChild( xml_node );
        }
        else
        {
          break;
        }

        // Did we get encoding info?
        if (    xml_encoding == MGZXML_ENCODING_UNKNOWN
            && xml_node->ToDeclaration() )
        {
          declaration* dec = xml_node->ToDeclaration();
          const char* enc = dec->Encoding();
          assert( enc );

          if ( *enc == 0 )
            xml_encoding = MGZXML_ENCODING_UTF8;
          else if ( StringEqual( enc, "UTF-8", true, MGZXML_ENCODING_UNKNOWN ) )
            xml_encoding = MGZXML_ENCODING_UTF8;
          else if ( StringEqual( enc, "UTF8", true, MGZXML_ENCODING_UNKNOWN ) )
            xml_encoding = MGZXML_ENCODING_UTF8;	// incorrect, but be nice
          else 
            xml_encoding = MGZXML_ENCODING_LEGACY;
        }

        p = SkipWhiteSpace( p, xml_encoding );
      }

      // Was this empty?
      if ( !firstChild ) {
        SetError( MGZXML_ERROR_DOCUMENT_EMPTY, 0, 0, xml_encoding );
        return 0;
      }

      // All is well.
      return p;
    }

    void document::SetError( int err, const char* pError, parsingData* data, encoding xml_encoding )
    {	
      // The first error in a chain is more accurate - don't set again!
      if ( error )
        return;

      assert( err > 0 && err < MGZXML_ERROR_STRING_COUNT );
      error   = true;
      errorId = err;
      errorDesc = errorString[ errorId ];

      errorLocation.Clear();
      if ( pError && data )
      {
        data->Stamp( pError, xml_encoding );
        errorLocation = data->Cursor();
      }
    }


    node* node::Identify( const char* p, encoding xml_encoding )
    {
      node* returnNode = 0;

      p = SkipWhiteSpace( p, xml_encoding );
      if( !p || !*p || *p != '<' )
      {
        return 0;
      }

      p = SkipWhiteSpace( p, xml_encoding );

      if ( !p || !*p )
      {
        return 0;
      }

      // What is this thing? 
      // - Elements start with a letter or underscore, but xml is reserved.
      // - Comments: <!--
      // - Decleration: <?xml
      // - Everthing else is unknown to glowxml.
      //

      const char* xmlHeader = { "<?xml" };
      const char* commentHeader = { "<!--" };
      const char* dtdHeader = { "<!" };
      const char* cdataHeader = { "<![CDATA[" };

      if ( StringEqual( p, xmlHeader, true, xml_encoding ) )
      {
#ifdef DEBUG_PARSER
        MGZXML_LOG( "XML parsing Declaration\n" );
#endif
        returnNode = new declaration();
      }
      else if ( StringEqual( p, commentHeader, false, xml_encoding ) )
      {
#ifdef DEBUG_PARSER
        MGZXML_LOG( "XML parsing Comment\n" );
#endif
        returnNode = new comment();
      }
      else if ( StringEqual( p, cdataHeader, false, xml_encoding ) )
      {
#ifdef DEBUG_PARSER
        MGZXML_LOG( "XML parsing CDATA\n" );
#endif
        text* xml_text = new text( "" );
        xml_text->SetCDATA( true );
        returnNode = xml_text;
      }
      else if ( StringEqual( p, dtdHeader, false, xml_encoding ) )
      {
#ifdef DEBUG_PARSER
        MGZXML_LOG( "XML parsing Unknown(1)\n" );
#endif
        returnNode = new unknown();
      }
      else if (    IsAlpha( *(p+1), xml_encoding )
          || *(p+1) == '_' )
      {
#ifdef DEBUG_PARSER
        MGZXML_LOG( "XML parsing Element\n" );
#endif
        returnNode = new element( "" );
      }
      else
      {
#ifdef DEBUG_PARSER
        MGZXML_LOG( "XML parsing Unknown(2)\n" );
#endif
        returnNode = new unknown();
      }

      if ( returnNode )
      {
        // Set the parent, so it can report errors
        returnNode->parent = this;
      }
      return returnNode;
    }

    void element::StreamIn (std::istream * in, MGZXML_STRING * tag)
    {
      // We're called with some amount of pre-parsing. That is, some of "this"
      // element is in "tag". Go ahead and stream to the closing ">"
      while( in->good() )
      {
        int c = in->get();
        if ( c <= 0 )
        {
          document* xml_document = GetDocument();
          if ( xml_document )
            xml_document->SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
          return;
        }
        (*tag) += (char) c ;

        if ( c == '>' )
          break;
      }

      if ( tag->length() < 3 ) return;

      // Okay...if we are a "/>" tag, then we're done. We've read a complete tag.
      // If not, identify and stream.

      if (    tag->at( tag->length() - 1 ) == '>' 
          && tag->at( tag->length() - 2 ) == '/' )
      {
        // All good!
        return;
      }
      else if ( tag->at( tag->length() - 1 ) == '>' )
      {
        // There is more. Could be:
        //		text
        //		cdata text (which looks like another node)
        //		closing tag
        //		another node.
        for ( ;; )
        {
          StreamWhiteSpace( in, tag );

          // Do we have text?
          if ( in->good() && in->peek() != '<' ) 
          {
            // Yep, text.
            text xml_text( "" );
            xml_text.StreamIn( in, tag );

            // What follows text is a closing tag or another node.
            // Go around again and figure it out.
            continue;
          }

          // We now have either a closing tag...or another node.
          // We should be at a "<", regardless.
          if ( !in->good() ) return;
          assert( in->peek() == '<' );
          int tagIndex = (int) tag->length();

          bool closingTag = false;
          bool firstCharFound = false;

          for( ;; )
          {
            if ( !in->good() )
              return;

            int c = in->peek();
            if ( c <= 0 )
            {
              document* xml_document = GetDocument();
              if ( xml_document )
                xml_document->SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
              return;
            }

            if ( c == '>' )
              break;

            *tag += (char) c;
            in->get();

            // Early out if we find the CDATA id.
            if ( c == '[' && tag->size() >= 9 )
            {
              size_t len = tag->size();
              const char* start = tag->c_str() + len - 9;
              if ( strcmp( start, "<![CDATA[" ) == 0 ) {
                assert( !closingTag );
                break;
              }
            }

            if ( !firstCharFound && c != '<' && !IsWhiteSpace( c ) )
            {
              firstCharFound = true;
              if ( c == '/' )
                closingTag = true;
            }
          }
          // If it was a closing tag, then read in the closing '>' to clean up the input stream.
          // If it was not, the streaming will be done by the tag.
          if ( closingTag )
          {
            if ( !in->good() )
              return;

            int c = in->get();
            if ( c <= 0 )
            {
              document* xml_document = GetDocument();
              if ( xml_document )
                xml_document->SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
              return;
            }
            assert( c == '>' );
            *tag += (char) c;

            // We are done, once we've found our closing tag.
            return;
          }
          else
          {
            // If not a closing tag, id it, and stream.
            const char* tagloc = tag->c_str() + tagIndex;
            node* xml_node = Identify( tagloc, MGZXML_DEFAULT_ENCODING );
            if ( !xml_node )
              return;
            xml_node->StreamIn( in, tag );
            delete xml_node;
            xml_node = 0;

            // No return: go around from the beginning: text, closing tag, or node.
          }
        }
      }
    }

    const char* element::Parse( const char* p, parsingData* data, encoding xml_encoding )
    {
      p = SkipWhiteSpace( p, xml_encoding );
      document* xml_document = GetDocument();

      if ( !p || !*p )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_PARSING_ELEMENT, 0, 0, xml_encoding );
        return 0;
      }

      if ( data )
      {
        data->Stamp( p, xml_encoding );
        location = data->Cursor();
      }

      if ( *p != '<' )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_PARSING_ELEMENT, p, data, xml_encoding );
        return 0;
      }

      p = SkipWhiteSpace( p+1, xml_encoding );

      // Read the name.
      const char* pErr = p;

      p = ReadName( p, &value, xml_encoding );
      if ( !p || !*p )
      {
        if ( xml_document )	xml_document->SetError( MGZXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, xml_encoding );
        return 0;
      }

      MGZXML_STRING endTag ("</");
      endTag += value;

      // Check for and read attributes. Also look for an empty
      // tag or an end tag.
      while ( p && *p )
      {
        pErr = p;
        p = SkipWhiteSpace( p, xml_encoding );
        if ( !p || !*p )
        {
          if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_ATTRIBUTES, pErr, data, xml_encoding );
          return 0;
        }
        if ( *p == '/' )
        {
          ++p;
          // Empty tag.
          if ( *p  != '>' )
          {
            if ( xml_document ) xml_document->SetError( MGZXML_ERROR_PARSING_EMPTY, p, data, xml_encoding );		
            return 0;
          }
          return (p+1);
        }
        else if ( *p == '>' )
        {
          // Done with attributes (if there were any.)
          // Read the value -- which can include other
          // elements -- read the end tag, and return.
          ++p;
          p = ReadValue( p, data, xml_encoding );		// Note this is an Element method, and will set the error if one happens.
          if ( !p || !*p ) {
            // We were looking for the end tag, but found nothing.
            // Fix for [ 1663758 ] Failure to report error on bad XML
            if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_END_TAG, p, data, xml_encoding );
            return 0;
          }

          // We should find the end tag now
          // note that:
          // </foo > and
          // </foo> 
          // are both valid end tags.
          if ( StringEqual( p, endTag.c_str(), false, xml_encoding ) )
          {
            p += endTag.length();
            p = SkipWhiteSpace( p, xml_encoding );
            if ( p && *p && *p == '>' ) {
              ++p;
              return p;
            }
            if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_END_TAG, p, data, xml_encoding );
            return 0;
          }
          else
          {
            if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_END_TAG, p, data, xml_encoding );
            return 0;
          }
        }
        else
        {
          // Try to read an attribute:
          attribute* attrib = new attribute();
          if ( !attrib )
          {
            return 0;
          }

          attrib->SetDocument( xml_document );
          pErr = p;
          p = attrib->Parse( p, data, xml_encoding );

          if ( !p || !*p )
          {
            if ( xml_document ) xml_document->SetError( MGZXML_ERROR_PARSING_ELEMENT, pErr, data, xml_encoding );
            delete attrib;
            return 0;
          }

          // Handle the strange case of double attributes:
          attribute* xml_node = xml_attributeSet.Find( attrib->NameTStr() );
          if ( xml_node )
          {
            if ( xml_document ) xml_document->SetError( MGZXML_ERROR_PARSING_ELEMENT, pErr, data, xml_encoding );
            delete attrib;
            return 0;
          }

          xml_attributeSet.Add( attrib );
        }
      }
      return p;
    }


    const char* element::ReadValue( const char* p, parsingData* data, encoding xml_encoding )
    {
      document* xml_document = GetDocument();

      // Read in text and elements in any order.
      const char* pWithWhiteSpace = p;
      p = SkipWhiteSpace( p, xml_encoding );

      while ( p && *p )
      {
        if ( *p != '<' )
        {
          // Take what we have, make a text element.
          text* textNode = new text( "" );

          if ( !textNode )
          {
            return 0;
          }

          if ( base::IsWhiteSpaceCondensed() )
          {
            p = textNode->Parse( p, data, xml_encoding );
          }
          else
          {
            // Special case: we want to keep the white space
            // so that leading spaces aren't removed.
            p = textNode->Parse( pWithWhiteSpace, data, xml_encoding );
          }

          if ( !textNode->Blank() )
            LinkEndChild( textNode );
          else
            delete textNode;
        } 
        else 
        {
          // We hit a '<'
          // Have we hit a new element or an end tag? This could also be
          // a text in the "CDATA" style.
          if ( StringEqual( p, "</", false, xml_encoding ) )
          {
            return p;
          }
          else
          {
            node* xml_node = Identify( p, xml_encoding );
            if ( xml_node )
            {
              p = xml_node->Parse( p, data, xml_encoding );
              LinkEndChild( xml_node );
            }				
            else
            {
              return 0;
            }
          }
        }
        pWithWhiteSpace = p;
        p = SkipWhiteSpace( p, xml_encoding );
      }

      if ( !p )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_ELEMENT_VALUE, 0, 0, xml_encoding );
      }	
      return p;
    }


    void unknown::StreamIn( std::istream * in, MGZXML_STRING * tag )
    {
      while ( in->good() )
      {
        int c = in->get();	
        if ( c <= 0 )
        {
          document* xml_document = GetDocument();
          if ( xml_document )
            xml_document->SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
          return;
        }
        (*tag) += (char) c;

        if ( c == '>' )
        {
          // All is well.
          return;		
        }
      }
    }


    const char* unknown::Parse( const char* p, parsingData* data, encoding xml_encoding )
    {
      document* xml_document = GetDocument();
      p = SkipWhiteSpace( p, xml_encoding );

      if ( data )
      {
        data->Stamp( p, xml_encoding );
        location = data->Cursor();
      }
      if ( !p || !*p || *p != '<' )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_PARSING_UNKNOWN, p, data, xml_encoding );
        return 0;
      }
      ++p;
      value = "";

      while ( p && *p && *p != '>' )
      {
        value += *p;
        ++p;
      }

      if ( !p )
      {
        if ( xml_document )	
          xml_document->SetError( MGZXML_ERROR_PARSING_UNKNOWN, 0, 0, xml_encoding );
      }
      if ( p && *p == '>' )
        return p+1;
      return p;
    }

    void comment::StreamIn( std::istream * in, MGZXML_STRING * tag )
    {
      while ( in->good() )
      {
        int c = in->get();	
        if ( c <= 0 )
        {
          document* xml_document = GetDocument();
          if ( xml_document )
            xml_document->SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
          return;
        }

        (*tag) += (char) c;

        if ( c == '>' 
            && tag->at( tag->length() - 2 ) == '-'
            && tag->at( tag->length() - 3 ) == '-' )
        {
          // All is well.
          return;		
        }
      }
    }


    const char* comment::Parse( const char* p, parsingData* data, encoding xml_encoding )
    {
      document* xml_document = GetDocument();
      value = "";

      p = SkipWhiteSpace( p, xml_encoding );

      if ( data )
      {
        data->Stamp( p, xml_encoding );
        location = data->Cursor();
      }
      const char* startTag = "<!--";
      const char* endTag   = "-->";

      if ( !StringEqual( p, startTag, false, xml_encoding ) )
      {
        if ( xml_document )
          xml_document->SetError( MGZXML_ERROR_PARSING_COMMENT, p, data, xml_encoding );
        return 0;
      }
      p += strlen( startTag );

      // [ 1475201 ] mgz::xml parses entities in comments
      // Oops - ReadText doesn't work, because we don't want to parse the entities.
      // p = ReadText( p, &value, false, endTag, false, xml_encoding );
      //
      // from the XML spec:
      /*
         [Definition: Comments may appear anywhere in a document outside other markup; in addition, 
         they may appear within the document type declaration at places allowed by the grammar. 
         They are not part of the document's character data; an XML processor MAY, but need not, 
         make it possible for an application to retrieve the text of comments. For compatibility, 
         the string "--" (double-hyphen) MUST NOT occur within comments.] Parameter entity 
         references MUST NOT be recognized within comments.

         An example of a comment:

         <!-- declarations for <head> & <body> -->
         */

      value = "";
      // Keep all the white space.
      while (	p && *p && !StringEqual( p, endTag, false, xml_encoding ) )
      {
        value.append( p, 1 );
        ++p;
      }
      if ( p && *p ) 
        p += strlen( endTag );

      return p;
    }


    const char* attribute::Parse( const char* p, parsingData* data, encoding xml_encoding )
    {
      p = SkipWhiteSpace( p, xml_encoding );
      if ( !p || !*p ) return 0;

      if ( data )
      {
        data->Stamp( p, xml_encoding );
        location = data->Cursor();
      }
      // Read the name, the '=' and the value.
      const char* pErr = p;
      p = ReadName( p, &name, xml_encoding );
      if ( !p || !*p )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_ATTRIBUTES, pErr, data, xml_encoding );
        return 0;
      }
      p = SkipWhiteSpace( p, xml_encoding );
      if ( !p || !*p || *p != '=' )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_ATTRIBUTES, p, data, xml_encoding );
        return 0;
      }

      ++p;	// skip '='
      p = SkipWhiteSpace( p, xml_encoding );
      if ( !p || !*p )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_ATTRIBUTES, p, data, xml_encoding );
        return 0;
      }

      const char* end;
      const char SINGLE_QUOTE = '\'';
      const char DOUBLE_QUOTE = '\"';

      if ( *p == SINGLE_QUOTE )
      {
        ++p;
        end = "\'";		// single quote in string
        p = ReadText( p, &value, false, end, false, xml_encoding );
      }
      else if ( *p == DOUBLE_QUOTE )
      {
        ++p;
        end = "\"";		// double quote in string
        p = ReadText( p, &value, false, end, false, xml_encoding );
      }
      else
      {
        // All attribute values should be in single or double quotes.
        // But this is such a common error that the parser will try
        // its best, even without them.
        value = "";
        while (    p && *p											// existence
            && !IsWhiteSpace( *p )								// whitespace
            && *p != '/' && *p != '>' )							// tag end
        {
          if ( *p == SINGLE_QUOTE || *p == DOUBLE_QUOTE ) {
            // [ 1451649 ] Attribute values with trailing quotes not handled correctly
            // We did not have an opening quote but seem to have a 
            // closing one. Give up and throw an error.
            if ( xml_document ) xml_document->SetError( MGZXML_ERROR_READING_ATTRIBUTES, p, data, xml_encoding );
            return 0;
          }
          value += *p;
          ++p;
        }
      }
      return p;
    }

    void text::StreamIn( std::istream * in, MGZXML_STRING * tag )
    {
      while ( in->good() )
      {
        int c = in->peek();	
        if ( !cdata && (c == '<' ) ) 
        {
          return;
        }
        if ( c <= 0 )
        {
          document* xml_document = GetDocument();
          if ( xml_document )
            xml_document->SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
          return;
        }

        (*tag) += (char) c;
        in->get();	// "commits" the peek made above

        if ( cdata && c == '>' && tag->size() >= 3 ) {
          size_t len = tag->size();
          if ( (*tag)[len-2] == ']' && (*tag)[len-3] == ']' ) {
            // terminator of cdata.
            return;
          }
        }    
      }
    }

    const char* text::Parse( const char* p, parsingData* data, encoding xml_encoding )
    {
      value = "";
      document* xml_document = GetDocument();

      if ( data )
      {
        data->Stamp( p, xml_encoding );
        location = data->Cursor();
      }

      const char* const startTag = "<![CDATA[";
      const char* const endTag   = "]]>";

      if ( cdata || StringEqual( p, startTag, false, xml_encoding ) )
      {
        cdata = true;

        if ( !StringEqual( p, startTag, false, xml_encoding ) )
        {
          if ( xml_document )
            xml_document->SetError( MGZXML_ERROR_PARSING_CDATA, p, data, xml_encoding );
          return 0;
        }
        p += strlen( startTag );

        // Keep all the white space, ignore the xml_encoding, etc.
        while (	   p && *p
            && !StringEqual( p, endTag, false, xml_encoding )
            )
        {
          value += *p;
          ++p;
        }

        MGZXML_STRING dummy; 
        p = ReadText( p, &dummy, false, endTag, false, xml_encoding );
        return p;
      }
      else
      {
        bool ignoreWhite = true;

        const char* end = "<";
        p = ReadText( p, &value, ignoreWhite, end, false, xml_encoding );
        if ( p && *p )
          return p-1;	// don't truncate the '<'
        return 0;
      }
    }

    void declaration::StreamIn( std::istream * in, MGZXML_STRING * tag )
    {
      while ( in->good() )
      {
        int c = in->get();
        if ( c <= 0 )
        {
          document* xml_document = GetDocument();
          if ( xml_document )
            xml_document->SetError( MGZXML_ERROR_EMBEDDED_NULL, 0, 0, MGZXML_ENCODING_UNKNOWN );
          return;
        }
        (*tag) += (char) c;

        if ( c == '>' )
        {
          // All is well.
          return;
        }
      }
    }

    const char* declaration::Parse( const char* p, parsingData* data, encoding _xml_encoding )
    {
      p = SkipWhiteSpace( p, _xml_encoding );
      // Find the beginning, find the end, and look for
      // the stuff in-between.
      document* xml_document = GetDocument();
      if ( !p || !*p || !StringEqual( p, "<?xml", true, _xml_encoding ) )
      {
        if ( xml_document ) xml_document->SetError( MGZXML_ERROR_PARSING_DECLARATION, 0, 0, _xml_encoding );
        return 0;
      }
      if ( data )
      {
        data->Stamp( p, _xml_encoding );
        location = data->Cursor();
      }
      p += 5;

      version = "";
      xml_encoding = "";
      standalone = "";

      while ( p && *p )
      {
        if ( *p == '>' )
        {
          ++p;
          return p;
        }

        p = SkipWhiteSpace( p, _xml_encoding );
        if ( StringEqual( p, "version", true, _xml_encoding ) )
        {
          attribute attrib;
          p = attrib.Parse( p, data, _xml_encoding );		
          version = attrib.Value();
        }
        else if ( StringEqual( p, "encoding", true, _xml_encoding ) )
        {
          attribute attrib;
          p = attrib.Parse( p, data, _xml_encoding );		
          xml_encoding = attrib.Value();
        }
        else if ( StringEqual( p, "standalone", true, _xml_encoding ) )
        {
          attribute attrib;
          p = attrib.Parse( p, data, _xml_encoding );		
          standalone = attrib.Value();
        }
        else
        {
          // Read over whatever it is.
          while( p && *p && *p != '>' && !IsWhiteSpace( *p ) )
            ++p;
        }
      }
      return 0;
    }

    bool text::Blank() const
    {
      for ( unsigned i=0; i<value.length(); i++ )
        if ( !IsWhiteSpace( value[i] ) )
          return false;
      return true;
    }
  }
}
