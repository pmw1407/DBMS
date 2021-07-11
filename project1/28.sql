SELECT T.name, AVG(C.level) AS average 
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE T.id = C.owner_id AND C.pid = P.id AND (P.type = 'Electric' OR P.type = 'Normal')
GROUP BY T.id
ORDER BY average;