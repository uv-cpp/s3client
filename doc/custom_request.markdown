# Creating custom requests

The provided functions and classes only support a basic subset of
the AWS; all the methods simply invoke the Send method and return
the parsed response.
The only value added by the current implementation is the parsing of the XML
response in cases where data is not returned in headers.
However, client code will most likely need to translate from the returned
objects to their own domain specific internal representation and it might
therefore be easier to just parse the XML directly.

Any request listed on the *AWS* website can be sent using this library by:

1. creating a function that returns
  - request parameters as ..
  - request body, if required
2. sending the request using the S3Api methods or the free function Send rquest
3. creating a function that returns the parsed response body and HTTP headers
  - HTTP headers are already parsed and stored into a `map` object
  - for parsing the returned XML you can use the provided parsing functions
    or use a library like pugixml (XPath compliant, value semantics, supports range based loops)
  - in addition to high-level parsing functions, tinyXML2 is included in the
    source tree, used internally, and can be used as well by client code

## Example 1: 

## Example 2:
