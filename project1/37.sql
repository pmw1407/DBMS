SELECT name, MAX(s)
FROM(SELECT T.name, SUM(C.level) AS s
FROM CatchedPokemon AS C, Trainer AS T
WHERE C.owner_id = T.id
GROUP BY T.id) AS result;