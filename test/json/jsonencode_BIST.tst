%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Unit tests for jsonencode()
%%
%% Code in libinterp/corefcn/jsonencode.cc
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Note: This script is intended to also run under Matlab to verify
%%       compatibility.  Preserve Matlab-formatting when making changes.

%% Some tests here are just the reverse of tests in jsondecode with
%% some modifications.

%%% Test 1: Encode logical and numeric scalars, NaN, NA, and Inf

%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode (logical (1)), 'true'));
%! assert (isequal (jsonencode (logical (0)), 'false'));
%! assert (isequal (jsonencode (50.025), '50.025'));
%! assert (isequal (jsonencode (NaN), 'null'));
%! assert (isequal (jsonencode (NA), 'null'));    % Octave-only test
%! assert (isequal (jsonencode (Inf), 'null'));
%! assert (isequal (jsonencode (-Inf), 'null'));

%% Customized encoding of Nan, NA, Inf, -Inf
%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode (NaN, 'ConvertInfAndNaN', true), 'null'));
%! % Octave-only test for NA
%! assert (isequal (jsonencode (NA, 'ConvertInfAndNaN', true), 'null'));
%! assert (isequal (jsonencode (Inf, 'ConvertInfAndNaN', true), 'null'));
%! assert (isequal (jsonencode (-Inf, 'ConvertInfAndNaN', true), 'null'));

%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode (NaN, 'ConvertInfAndNaN', false), 'NaN'));
%! % Octave-only test for NA
%! assert (isequal (jsonencode (NA, 'ConvertInfAndNaN', false), 'NaN'));
%! assert (isequal (jsonencode (Inf, 'ConvertInfAndNaN', false), 'Infinity'));
%! assert (isequal (jsonencode (-Inf, 'ConvertInfAndNaN', false), '-Infinity'));

%%% Test 2: encode character vectors and arrays

%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode (''), '""'));
%! assert (isequal (jsonencode ('hello there'), '"hello there"'));
%! assert (isequal (jsonencode (['foo'; 'bar']), '["foo","bar"]'));
%! assert (isequal (jsonencode (['foo', 'bar'; 'foo', 'bar']), ...
%!                  '["foobar","foobar"]'));

%% Escape characters inside single-quoted and double-quoted strings
%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode ('\0\a\b\t\n\v\f\r'), ...
%!                              '"\\0\\a\\b\\t\\n\\v\\f\\r"'));
%! % FIXME: Matlab produces a double-escaped string as above.
%! assert (isequal (jsonencode ("\a\b\t\n\v\f\r"), ...
%!                              '"\u0007\b\t\n\u000B\f\r"'));

%!testif HAVE_RAPIDJSON
%! data = [[['foo'; 'bar']; ['foo'; 'bar']], [['foo'; 'bar']; ['foo'; 'bar']]];
%! exp  = '["foofoo","barbar","foofoo","barbar"]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = cat (3, ['a', 'b'; 'c', 'd'], ['e', 'f'; 'g', 'h']);
%! exp  = '[["ab","ef"],["cd","gh"]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Try different dimensions for the array
%!testif HAVE_RAPIDJSON
%! data = cat (3, ['a', 'b'; 'c', 'd'; '1', '2'], ...
%!                ['e', 'f'; 'g', 'h'; '3', '4']);
%! exp  = '[["ab","ef"],["cd","gh"],["12","34"]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Try higher dimensions for the array
%!testif HAVE_RAPIDJSON
%! charmat1 = cat (3, ['1', '3'; '5', '7'; '9', 'e'; 'f', 'g'], ...
%!                    ['2', '4'; '6', '8'; 'a', 'b'; 'c', 'd']);
%! charmat2 = cat (3, ['1', '3'; '5', '7'; '9', 'e'; 'f', 'g'], ...
%!                    ['2', '4'; '6', '8'; 'a', 'b'; 'c', 'd']);
%! data = cat (4, charmat1, charmat2);
%! exp  = [ '[[["13","13"],["24","24"]],[["57","57"],["68","68"]],', ...
%!          '[["9e","9e"],["ab","ab"]],[["fg","fg"],["cd","cd"]]]' ];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Try different dimensions for an array with one of its dimensions equals one
%!testif HAVE_RAPIDJSON
%! data = cat (4, ['a'; 'b'], ['c'; 'd']);
%! exp  = '[[["a","c"]],[["b","d"]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% High dimension, but still a vector, is reduced to a vector
%!testif HAVE_RAPIDJSON
%! data = cat (8, ['a'], ['c']);
%! exp  = '"ac"';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = cat (8, ['a'; 'b'; '1'], ['c'; 'd'; '2']);
%! exp  = '[[[[[[["a","c"]]]]]],[[[[[["b","d"]]]]]],[[[[[["1","2"]]]]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%%% Test 3: encode numeric and logical arrays (with NaN and Inf)

%% Test simple vectors
%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode ([]), '[]'));
%! assert (isequal (jsonencode ([1, 2, 3, 4]), '[1,2,3,4]'));
%! assert (isequal (jsonencode ([true; false; true]), '[true,false,true]'));

%% Test arrays
%!testif HAVE_RAPIDJSON
%! data = [1, NaN; 3, 4];
%! exp  = '[[1,null],[3,4]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = cat (3, [NaN, 3; 5, Inf], [2, 4; -Inf, 8]);
%! exp  = '[[[null,2],[3,4]],[[5,null],[null,8]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Customized encoding of Nan, Inf, -Inf
%!testif HAVE_RAPIDJSON
%! data = cat (3, [1, NaN; 5, 7], [2, Inf; 6, -Inf]);
%! exp  = '[[[1,2],[NaN,Infinity]],[[5,6],[7,-Infinity]]]';
%! obs  = jsonencode (data, 'ConvertInfAndNaN', false);
%! assert (isequal (obs, exp));

%% Try different dimensions for the array
%!testif HAVE_RAPIDJSON
%! data = cat (3, [1, 3; 5, 7], [2, 4; 6, 8], [-1, NaN; Inf, -Inf]);
%! exp  = '[[[1,2,-1],[3,4,NaN]],[[5,6,Infinity],[7,8,-Infinity]]]';
%! obs  = jsonencode (data, 'ConvertInfAndNaN', false);
%! assert (isequal (obs, exp));

%% Try different dimensions for the array with one of its dimensions equals one
%!testif HAVE_RAPIDJSON
%! data = cat (3, [1; 7; 11], [4; 8; 12]);
%! exp  = '[[[1,4]],[[7,8]],[[11,12]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! array1 = cat (3, [5, 7], [2, 4]);
%! array2 = cat (3, [-1, -3], [-2, -4]);
%! data = cat (4, array1, array2);
%! exp  = '[[[[5,-1],[2,-2]],[[7,-3],[4,-4]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = cat (4, [1, 3; 5, 7], [-1, -3; -5, -7]);
%! exp  = '[[[[1,-1]],[[3,-3]]],[[[5,-5]],[[7,-7]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% High-dimension vector is reduced to just a vector
%!testif HAVE_RAPIDJSON
%! data = ones ([1, 1, 1, 1, 1, 6]);
%! exp  = '[1,1,1,1,1,1]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 2, 2, 2, 2]);
%! exp  = '[[[[[1,1],[1,1]],[[1,1],[1,1]]],[[[1,1],[1,1]],[[1,1],[1,1]]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 2, 2, 1, 2]);
%! exp  = '[[[[[1,1]],[[1,1]]],[[[1,1]],[[1,1]]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 2, 1, 2, 1, 2]);
%! exp  = '[[[[[[1,1]],[[1,1]]]],[[[[1,1]],[[1,1]]]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 1, 2, 1, 2, 1, 2]);
%! exp  = '[[[[[[[1,1]],[[1,1]]]],[[[[1,1]],[[1,1]]]]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 2, 2, 1, 1, 2]);
%! exp  = '[[[[[[1,1]]],[[[1,1]]]],[[[[1,1]]],[[[1,1]]]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 2, 1, 3, 1, 1, 1, 2]);
%! exp  = ['[[[[[[[[1,1]]]],[[[[1,1]]]],[[[[1,1]]]]]],[[[[[[1,1]]]],', ...
%!         '[[[[1,1]]]],[[[[1,1]]]]]]]]'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 1, 1, 1, 2, 1, 1, 1, 2]);
%! exp  = '[[[[[[[[[1,1]]]],[[[[1,1]]]]]]]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 3, 2, 1, 1, 2, 1, 2, 2]);
%! exp  = ['[[[[[[[[[1,1],[1,1]]],[[[1,1],[1,1]]]]]],[[[[[[1,1],', ...
%!         '[1,1]]],[[[1,1],[1,1]]]]]]],[[[[[[[1,1],[1,1]]],[[[1,', ...
%!         '1],[1,1]]]]]],[[[[[[1,1],[1,1]]],[[[1,1],[1,1]]]]]]],', ...
%!         '[[[[[[[1,1],[1,1]]],[[[1,1],[1,1]]]]]],[[[[[[1,1],[1,', ...
%!         '1]]],[[[1,1],[1,1]]]]]]]]]'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = ones ([1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 2]);
%! exp  =  ['[[[[[[[[[[[[[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]],', ...
%!          '[[[[[[[[1,1]]]]]]]]],[[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]]]]', ...
%!          ']]],[[[[[[[[1,1]]]]]]]]]],[[[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]', ...
%!          ']]]]]]],[[[[[[[[1,1]]]]]]]]],[[[[[[[[[1,1]]]]]]]],[[[[[[', ...
%!          '[[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]]]]]]],[[[[[[[[[[[[[1,1]', ...
%!          ']]]]]]],[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]]],[[[[[[[[', ...
%!          '[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]]]],[[[', ...
%!          '[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]]],', ...
%!          '[[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]]]]]]],[[[[[[[[1,1]]', ...
%!          ']]]]]]]]]]]]]]]]]]'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Try higher dimensions for the array
%!testif HAVE_RAPIDJSON
%! var1 = cat (3, [1, 3; 5, 7; 9, 11; 13, 15], [2, 4; 6, 8; 10, 12; 14, 16]);
%! var2 = cat (3, [-1, -3; -5, -7; -9, -11; -13, -15], ...
%!                [-2, -4; -6, -8; -10, -12; -14, -16]);
%! data = cat (4, var1, var2);
%! exp  = ['[[[[1,-1],[2,-2]],[[3,-3],[4,-4]]],[[[5,-5],[6,-6]],[[7,-7],', ...
%!         '[8,-8]]],[[[9,-9],[10,-10]],[[11,-11],[12,-12]]],', ...
%!         '[[[13,-13],[14,-14]],[[15,-15],[16,-16]]]]'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Try logical array (tests above were all with numeric data)

%% 2-D logical array
%!testif HAVE_RAPIDJSON
%! data = [true, false; true, false; true, false];
%! exp  = '[[true,false],[true,false],[true,false]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% N-D logical array
%!testif HAVE_RAPIDJSON <*59198>
%! data = true (2,2,2);
%! data(1,1,2) = false;
%! exp  = '[[[true,false],[true,true]],[[true,true],[true,true]]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%%% Test 4: encode containers.Map

%% KeyType must be char to encode objects of containers.Map
%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode (containers.Map ('1', [1, 2, 3])), ...
%!                  '{"1":[1,2,3]}'));

%!testif HAVE_RAPIDJSON
%! data = containers.Map ({'foo'; 'bar'; 'baz'}, [1, 2, 3]);
%! exp  = '{"bar":2,"baz":3,"foo":1}';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON
%! data = containers.Map ({'foo'; 'bar'; 'baz'}, ...
%!                        {{1, 'hello', NaN}, true, [2, 3, 4]});
%! exp  = '{"bar":true,"baz":[2,3,4],"foo":[1,"hello",NaN]}';
%! obs  = jsonencode (data, 'ConvertInfAndNaN', false);
%! assert (isequal (obs, exp));

%%% Test 5: encode scalar structs

%% Check the encoding of Boolean, Number, and String values inside a struct
%!testif HAVE_RAPIDJSON
%! data = struct ('number', 3.14, 'string', 'foobar', 'boolean', false);
%! exp  = '{"number":3.14,"string":"foobar","boolean":false}';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Check the encoding of NaN, NA, Inf, and -Inf values inside a struct
%!testif HAVE_RAPIDJSON
%! % Octave-only test because of NA value
%! data = struct ('numericArray', [7, NaN, NA, Inf, -Inf]);
%! exp  = '{"numericArray":[7,null,null,null,null]}';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Customized encoding of Nan, NA, Inf, -Inf
%!testif HAVE_RAPIDJSON
%! data = struct ('numericArray', [7, NaN, NA, Inf, -Inf]);
%! exp  = '{"numericArray":[7,NaN,NaN,Infinity,-Infinity]}';
%! obs  = jsonencode (data, 'ConvertInfAndNaN', false);
%! assert (isequal (obs, exp));

%% Check the encoding of structs inside a struct
%!testif HAVE_RAPIDJSON
%! data = struct ('object', struct ('field1', 1, 'field2', 2, 'field3', 3));
%! exp  = '{"object":{"field1":1,"field2":2,"field3":3}}';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Check the encoding of empty structs and empty arrays inside a struct
%!testif HAVE_RAPIDJSON
%! data = struct ('a', Inf, 'b', [], 'c', struct ());
%! exp  = '{"a":null,"b":[],"c":{}}';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% a big test
%!testif HAVE_RAPIDJSON
%! var1 = struct ('para', ['A meta-markup language, used to create ' ...
%!                         'markup languages such as DocBook.'], ...
%!                'GlossSeeAlso', {{'GML'; 'XML'}});
%! var2 = struct ('ID', 'SGML', 'SortAs', 'SGML', ...
%!                'GlossTerm', 'Standard Generalized Markup Language', ...
%!                'Acronym', 'SGML', 'Abbrev', 'ISO 8879:1986', ...
%!                'GlossDef', var1, 'GlossSee', 'markup');
%! data  = struct ('glossary', ...
%!                struct ('title', 'example glossary', ...
%!                        'GlossDiv', struct ('title', 'S', ...
%!                                            'GlossList', ...
%!                                            struct ('GlossEntry', var2))));
%! exp = ['{' , ...
%!     '"glossary":{', ...
%!         '"title":"example glossary",', ...
%! 		'"GlossDiv":{', ...
%!             '"title":"S",', ...
%! 			'"GlossList":{', ...
%!                 '"GlossEntry":{', ...
%!                     '"ID":"SGML",', ...
%! 					'"SortAs":"SGML",', ...
%! 					'"GlossTerm":"Standard Generalized Markup Language",', ...
%! 					'"Acronym":"SGML",', ...
%! 					'"Abbrev":"ISO 8879:1986",', ...
%! 					'"GlossDef":{', ...
%!                         '"para":"A meta-markup language, ', ...
%!                         'used to create markup languages such as DocBook.",', ...
%! 						'"GlossSeeAlso":["GML","XML"]', ...
%!                     '},', ...
%! 					'"GlossSee":"markup"', ...
%!                 '}', ...
%!             '}', ...
%!         '}', ...
%!     '}', ...
%! '}'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%%% Test 6: Encode struct arrays

%!testif HAVE_RAPIDJSON
%! data = struct ('structarray', struct ('a', {1; 3}, 'b', {2; 4}));
%! exp  = '{"structarray":[{"a":1,"b":2},{"a":3,"b":4}]}';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON <*63622>
%! data = struct ('z', {});
%! exp  = '[]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));
%
%!testif HAVE_RAPIDJSON <*63622>
%! data.a = struct ('z', {});
%! data.b = 1;
%! exp  = '{"a":[],"b":1}';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% another big Test
%!testif HAVE_RAPIDJSON
%! var1 = struct ('id', {0; 1; 2}, 'name', {'Collins'; 'Hays'; 'Griffin'});
%! var2 = struct ('id', {0; 1; 2}, 'name', {'Osborn'; 'Mcdowell'; 'Jewel'});
%! var3 = struct ('id', {0; 1; 2}, 'name', {'Socorro'; 'Darla'; 'Leanne'});
%! data = struct (...
%!   'x_id', {'5ee28980fc9ab3'; '5ee28980dd7250'; '5ee289802422ac'}, ...
%!   'index', {0; 1; 2}, ...
%!   'guid', {'b229d1de-f94a'; '39cee338-01fb'; '3db8d55a-663e'}, ...
%!   'latitude', {-17.124067; 13.205994; -35.453456}, ...
%!   'longitude', {-61.161831; -37.276231; 14.080287}, ...
%!   'friends', {var1; var2; var3});
%! exp  = ['[', ...
%!   '{', ...
%!     '"x_id":"5ee28980fc9ab3",', ...
%!     '"index":0,', ...
%!     '"guid":"b229d1de-f94a",', ...
%!     '"latitude":-17.124067,', ...
%!     '"longitude":-61.161831,', ...
%!     '"friends":[', ...
%!       '{', ...
%!         '"id":0,', ...
%!         '"name":"Collins"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":1,', ...
%!         '"name":"Hays"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":2,', ...
%!         '"name":"Griffin"', ...
%!       '}', ...
%!     ']', ...
%!   '},', ...
%!   '{', ...
%!     '"x_id":"5ee28980dd7250",', ...
%!     '"index":1,', ...
%!     '"guid":"39cee338-01fb",', ...
%!     '"latitude":13.205994,', ...
%!     '"longitude":-37.276231,', ...
%!     '"friends":[', ...
%!       '{', ...
%!         '"id":0,', ...
%!         '"name":"Osborn"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":1,', ...
%!         '"name":"Mcdowell"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":2,', ...
%!         '"name":"Jewel"', ...
%!       '}', ...
%!     ']', ...
%!   '},', ...
%!   '{', ...
%!     '"x_id":"5ee289802422ac",', ...
%!     '"index":2,', ...
%!     '"guid":"3db8d55a-663e",', ...
%!     '"latitude":-35.453456,', ...
%!     '"longitude":14.080287,', ...
%!     '"friends":[', ...
%!       '{', ...
%!         '"id":0,', ...
%!         '"name":"Socorro"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":1,', ...
%!         '"name":"Darla"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":2,', ...
%!         '"name":"Leanne"', ...
%!       '}', ...
%!     ']', ...
%!   '}', ...
%! ']'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%%% Test 7: encode cell arrays

%!testif HAVE_RAPIDJSON
%! assert (isequal (jsonencode ({}), '[]'));
%! assert (isequal (jsonencode ({5}), '[5]'));
%! assert (isequal (jsonencode ({'hello there'}), '["hello there"]'));

%% Logical cell arrays
%!testif HAVE_RAPIDJSON
%! data = {'true', 'true'; 'false', 'true'};
%! exp  = '["true","false","true","true"]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Cell array of character vectors
%!testif HAVE_RAPIDJSON
%! data = {'foo'; 'bar'; {'foo'; 'bar'}};
%! exp  = '["foo","bar",["foo","bar"]]';
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% cell array of structs & a big test
%!testif HAVE_RAPIDJSON
%! var1 = struct ('x_id', '5ee28980fc9ab3', 'index', 0, ...
%!                'guid', 'b229d1de-f94a', 'latitude', -17.124067, ...
%!                'longitude', -61.161831, ...
%!                'friends', struct ('id', {0; 1; 2}, ...
%!                                   'name', {'Collins'; 'Hays'; 'Griffin'}));
%! var2 = struct ('numericArray', {{'str'; 5; []}}, ...
%!                'nonnumericArray', {[1; 2; NaN]});
%! var3 = struct ('firstName', 'John', 'lastName', 'Smith', 'age', 25, ...
%!                'address', ...
%!                struct ('streetAddress', '21 2nd Street', ...
%!                        'city', 'New York', 'state', 'NY'), ...
%!                'phoneNumber', ...
%!                struct ('type', 'home', 'number', '212 555-1234'));
%! data = {var1; var2; var3};
%! exp  = ['[', ...
%!   '{', ...
%!     '"x_id":"5ee28980fc9ab3",', ...
%!     '"index":0,', ...
%!     '"guid":"b229d1de-f94a",', ...
%!     '"latitude":-17.124067,', ...
%!     '"longitude":-61.161831,', ...
%!     '"friends":[', ...
%!       '{', ...
%!         '"id":0,', ...
%!         '"name":"Collins"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":1,', ...
%!         '"name":"Hays"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":2,', ...
%!         '"name":"Griffin"', ...
%!       '}', ...
%!     ']', ...
%!   '},', ...
%!   '{"numericArray":["str",5,[]],"nonnumericArray":[1,2,null]},', ...
%!   '{', ...
%!      '"firstName":"John",', ...
%!      '"lastName":"Smith",', ...
%!      '"age":25,', ...
%!      '"address":', ...
%!      '{', ...
%!          '"streetAddress":"21 2nd Street",', ...
%!          '"city":"New York",', ...
%!          '"state":"NY"', ...
%!      '},', ...
%!      '"phoneNumber":', ...
%!          '{', ...
%!            '"type":"home",', ...
%!            '"number":"212 555-1234"', ...
%!          '}', ...
%!  '}]'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% cell array of diferrent types & Customized encoding of Nan, Inf, -Inf
%!testif HAVE_RAPIDJSON
%! var =  struct ('x_id', '5ee28980dd7250', 'index', 1, ...
%!                'guid', '39cee338-01fb', 'latitude', 13.205994, ...
%!                'longitude', -37.276231, ...
%!                'friends', struct ('id', {0; 1; 2}, ...
%!                                   'name', {'Osborn'; 'Mcdowell'; 'Jewel'}));
%! data = {NaN; true; Inf; 2531.023; 'hello there'; var};
%! exp  = ['[NaN,true,Infinity,2531.023,"hello there",', ...
%!   '{', ...
%!     '"x_id":"5ee28980dd7250",', ...
%!     '"index":1,', ...
%!     '"guid":"39cee338-01fb",', ...
%!     '"latitude":13.205994,', ...
%!     '"longitude":-37.276231,', ...
%!     '"friends":[', ...
%!       '{', ...
%!         '"id":0,', ...
%!         '"name":"Osborn"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":1,', ...
%!         '"name":"Mcdowell"', ...
%!       '},', ...
%!       '{', ...
%!         '"id":2,', ...
%!         '"name":"Jewel"', ...
%!       '}', ...
%!     ']', ...
%!   '}]'];
%! obs  = jsonencode (data, 'ConvertInfAndNaN', false);
%! assert (isequal (obs, exp));

%% a big example
%!testif HAVE_RAPIDJSON
%! var1 = struct ('x_id', '5ee28980fc9ab3', 'index', 0, ...
%!                'guid', 'b229d1de-f94a', 'latitude', -17.124067, ...
%!                'longitude', -61.161831, ...
%!                'friends', struct ('id', {0; 1; 2}, ...
%!                                   'name', {'Collins'; 'Hays'; 'Griffin'}));
%! var2 = struct ('numericArray', {{'str'; 5; -Inf}}, ...
%!                'nonnumericArray', {[1; 2; NaN]});
%! var3 = struct ('firstName', 'John', 'lastName', 'Smith', 'age', 25, ...
%!                'address', ...
%!                struct ('streetAddress', '21 2nd Street', ...
%!                        'city', 'New York', 'state', 'NY'), ...
%!                'phoneNumber', ...
%!                struct ('type', 'home', 'number', '212 555-1234'));
%! data = {{'str'; Inf; {}}; [1; 2; NaN]; {'foo'; 'bar'; {'foo'; 'bar'}};
%!        cat(3, [1, 3; 5, 7], [2, 4; 6, 8]); {var1; var2 ;var3}};
%! exp  = ['[["str",null,[]],[1,2,null],["foo","bar",["foo","bar"]],', ...
%!   '[[[1,2],[3,4]],[[5,6],[7,8]]],' , ...
%!   '[', ...
%!     '{', ...
%!       '"x_id":"5ee28980fc9ab3",', ...
%!       '"index":0,', ...
%!       '"guid":"b229d1de-f94a",', ...
%!       '"latitude":-17.124067,', ...
%!       '"longitude":-61.161831,', ...
%!       '"friends":[', ...
%!         '{', ...
%!           '"id":0,', ...
%!           '"name":"Collins"', ...
%!         '},', ...
%!         '{', ...
%!           '"id":1,', ...
%!           '"name":"Hays"', ...
%!         '},', ...
%!         '{', ...
%!           '"id":2,', ...
%!           '"name":"Griffin"', ...
%!         '}', ...
%!       ']', ...
%!     '},', ...
%!     '{"numericArray":["str",5,null],"nonnumericArray":[1,2,null]},', ...
%!     '{', ...
%!        '"firstName":"John",', ...
%!        '"lastName":"Smith",', ...
%!        '"age":25,', ...
%!        '"address":', ...
%!        '{', ...
%!            '"streetAddress":"21 2nd Street",', ...
%!            '"city":"New York",', ...
%!            '"state":"NY"', ...
%!        '},', ...
%!        '"phoneNumber":', ...
%!            '{', ...
%!              '"type":"home",', ...
%!              '"number":"212 555-1234"', ...
%!            '}', ...
%!    '}]]'];
%! obs  = jsonencode (data);
%! assert (isequal (obs, exp));

%% Just basic tests to ensure option "PrettyPrint" is functional.
%!testif HAVE_RAPIDJSON_PRETTYWRITER
%! data = {'Hello'; 'World!'};
%! exp  = do_string_escapes ([ '[\n', ...
%!                             '  "Hello",\n', ...
%!                             '  "World!"\n', ...
%!                             ']' ]);
%! obs  = jsonencode (data, 'PrettyPrint', true);
%! assert (isequal (obs, exp));
%!
%! exp  = '["Hello","World!"]';
%! obs  = jsonencode (data, 'PrettyPrint', false);
%! assert (isequal (obs, exp));

%!testif HAVE_RAPIDJSON_PRETTYWRITER
%! data = [1, 2; 3, 4];
%! exp  = do_string_escapes ([ ...
%! '[\n'                       ...
%! '  [\n'                   ...
%! '    1,\n'              ...
%! '    2\n'               ...
%! '  ],\n'                  ...
%! '  [\n'                   ...
%! '    3,\n'              ...
%! '    4\n'               ...
%! '  ]\n'                   ...
%! ']' ]);
%! obs  = jsonencode (data, 'PrettyPrint', true);
%! assert (isequal (obs, exp));
%!
%! exp  = '[[1,2],[3,4]]';
%! obs  = jsonencode (data, 'PrettyPrint', false);
%! assert (isequal (obs, exp));
