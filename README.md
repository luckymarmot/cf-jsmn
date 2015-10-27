# cf-jsmn

[![CI Status](http://img.shields.io/travis/luckymarmot/cf-jsmn.svg?style=flat)](https://travis-ci.org/Micha Mazaheri/cf-jsmn)
[![Version](https://img.shields.io/cocoapods/v/cf-jsmn.svg?style=flat)](http://cocoapods.org/pods/cf-jsmn)
[![License](https://img.shields.io/cocoapods/l/cf-jsmn.svg?style=flat)](http://cocoapods.org/pods/cf-jsmn)
[![Platform](https://img.shields.io/cocoapods/p/cf-jsmn.svg?style=flat)](http://cocoapods.org/pods/cf-jsmn)

A Core Foundation compatible version of the awesome lightweight JSON parser [jsmn](https://github.com/zserge/jsmn), with support for unicode strings.


## Installation

cf-jsmn is available through [CocoaPods](http://cocoapods.org). To install
it, simply add the following line to your Podfile:

```ruby
pod "cf-jsmn"
```

## Example

```objectivec
// init the jsmn parser
jsmn_parser parser;
jsmn_init(&parser);
unsigned int numTokens = 65536; // an arbitrary size to start with
jsmntok_t* tokens = malloc(sizeof(jsmntok_t) * numTokens);

// get an "inline buffer" of UniChar from the CFString
CFIndex length = CFStringGetLength((__bridge CFStringRef)string);
CFStringInlineBuffer inlineBuffer;
CFStringInitInlineBuffer((__bridge CFStringRef)string, &inlineBuffer, CFRangeMake(0, length));

// loop until we have the right amount of tokens
while (YES) {

  // Parse
  jsmnerr_t error = jsmn_parse(&parser, &inlineBuffer, length, tokens, numTokens);

  // In no error, we're good
  if (error == JSMN_SUCCESS) {
    break;
  }
  // If not enough memory, realloc twice bigger
  else if (error == JSMN_ERROR_NOMEM) {
    numTokens *= 2;
    tokens = realloc(tokens, sizeof(jsmntok_t) * numTokens);
  }
  // If another error, fail
  else {
    // fail gracefully...
    break;
  }
}

// Loop over tokens
for (unsigned int i = 0; i < parser.toknext; i++) {

  int tk_start = tokens[i].start;
  int tk_end = tokens[i].end;
  NSRange range = NSMakeRange(tk_start, tk_end - tk_start);

  switch (tokens[i].type) {
    case JSMN_PRIMITIVE:
      break;
    case JSMN_STRING:
      break;
    case JSMN_OBJECT:
      break;
    case JSMN_ARRAY:
      break;
  }
}
```

## License

This software is distributed under [MIT license](http://www.opensource.org/licenses/mit-license.php), so feel free to integrate it in your commercial products.
