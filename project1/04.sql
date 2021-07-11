SELECT nickname
FROM CatchedPokemon as C, Pokemon as P
WHERE C.pid = P.id AND C.level >= 50
ORDER BY nickname;