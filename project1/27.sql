SELECT name, maxlv
FROM
(SELECT T.name, COUNT(C.pid) AS num, MAX(C.level) AS maxlv
FROM CatchedPokemon AS C, Trainer AS T
WHERE C.owner_id = T.id
GROUP BY T.id) AS result
WHERE num >= 4
ORDER BY name;