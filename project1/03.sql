SELECT AVG(level)
FROM CatchedPokemon AS C, Trainer AS T, Pokemon AS P
WHERE T.id = C.owner_id AND C.pid = P.id
  AND T.hometown = 'Sangnok City' AND P.type = 'Electric';