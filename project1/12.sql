SELECT DISTINCT P.name, P.type
FROM CatchedPokemon as C, Pokemon as P 
WHERE C.pid = P.id AND C.level >= 30
ORDER BY P.name;