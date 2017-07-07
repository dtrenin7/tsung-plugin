-module(csv_reader_nif).
-author('dtrenin7 @ gmail.com').

-export([getline/1, gc/1]).

-ifndef(PRINT).
-define(PRINT(Var), io:format("DEBUG: ~p:~p - ~p~n~n ~p~n~n", [?MODULE, ?LINE, ??Var, Var])).
-endif.

-on_load(init/0).

init() ->
   ok = erlang:load_nif("/usr/bin/csv_reader_nif", 0),
   ?PRINT(ok).

getline(Content_) ->
%    ["QUJDCg==", "QUJDCg==", "QUJDCg=="].
    exit(nif_library_not_loaded).

gc(Tick) ->
    case Tick rem 60 of
        0 -> erlang:garbage_collect(self());
        _ -> ok
    end.
