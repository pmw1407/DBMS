SELECT COUNT(numoftypes)
FROM (SELECT COUNT(type) AS numoftypes
FROM Gym as G, CatchedPokemon as C, Pokemon as P
WHERE G.leader_id = C.owner_id AND G.city = 'Sangnok City' AND C.pid = P.id
GROUP BY P.type) AS result;