SELECT AVG(level)
FROM CatchedPokemon AS C, Trainer as T
WHERE C.owner_id = T.id and T.name = 'Red';