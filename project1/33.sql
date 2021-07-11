SELECT SUM(C.level)
FROM CatchedPokemon AS C, Trainer AS T
WHERE C.owner_id = T.id AND T.name = 'Matis';
