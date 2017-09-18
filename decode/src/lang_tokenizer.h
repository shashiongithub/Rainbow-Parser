#ifndef LANG_TOKENIZER_H_
#define LANG_TOKENIZER_H_

#include <regex>
#include <string>

using namespace std;

class lang_tokenizer
{
public:
	typedef basic_regex<char> regex;
	
	enum
	{
		LEFT_SQUARE_BRACKET,
		RIGHT_SQUARE_BRACKET,
		IDENTIFIER,
		NUMBER
	} token_type;
	
	void tokenize_string(string line)
	{
		regex pat("^([a-z])" );
		smatch what;

		while (line.length() > 0) {
			
			if (regex_match(line, what, pat))
			{
				cerr<<what[0]<<endl;
				line.erase(0, what[0].length());
			}
		}
		
		// TODO
		
		// set all the regular expressions
		
		// check for all of them in the string
		
		// if there is more than one which starts at the current pointer, say there is an ambiguity.
		
		// otherwise, proceed to the end of the match and repeat
		
		// I don't think we need an iterator in this case
		
		// there is an example FLEX code on Wikipedia, it can help
	}
};

#endif /*LANG_TOKENIZER_H_*/


// http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=11&ved=0CCYQFjAAOAo&url=http%3A%2F%2Fnwcpp.org%2Ftalks%2F2007%2FText_Processing_With_Boost.ppt&ei=q5sYU4PgNcni0gG0zYHYBg&usg=AFQjCNGcUK4Wmsuxc8imm6YRQRcrukQsCQ&sig2=lAahTvn3tKlKZnV4_4CtqQ&bvm=bv.62577051,d.dmQ&cad=rja
