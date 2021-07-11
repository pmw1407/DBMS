SELECT DISTINCT T.name
FROM Trainer AS T, CatchedPokemon AS C
WHERE T.id = C.owner_id AND C.level <= 10
ORDER BY T.name;