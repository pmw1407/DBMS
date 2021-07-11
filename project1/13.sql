SELECT P.name, P.id
FROM Trainer as T, CatchedPokemon as C, Pokemon as P
WHERE T.id = C.owner_id AND P.id = C.pid AND T.hometown = 'Sangnok City'
ORDER BY P.id;