# Logger
Simple but powerful logging functionality

Only 4 levels exist instead of the usual 5:
* `ERROR`: Something unexpected and incorrect happened, these must be detailed and descriptive
  with source location, general state information and reasoning for error
* `WARN`: Something that _could_ lead to an error or is not ideal, but is not an actual error,
  for example calling `init()` twice, or an irregular parameter such as a zero sized list,
  these should also be detailed with reasoning
* `INFO`: Human readable simple information regarding what's happening, usually at important
  events such as Wifi initialization begun and Wifi connected etc. This is the default level
  and should be as accessible and friendly (while still being informative) as possible
* `VERBOSE`: Excessively detailed information with source code location and as much info about
  state as possible/needed such as function parameters and local variables etc. The more details,
  the better.

`DEBUG` was removed as it seemed generally unnecessary and too close to `INFO` but not as detailed
as `VERBOSE` which should be the best level for diagnosing issues
