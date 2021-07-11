SELECT T.name
FROM CatchedPokemon AS C, Trainer AS T
WHERE C.owner_id = T.id AND C.id <> ANY(
  SELECT id
  FROM CatchedPokemon
  WHERE pid = C.pid AND owner_id = C.owner_id)
GROUP BY T.id
ORDER BY T.name;