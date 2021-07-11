SELECT DISTINCT P.name
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE T.id = C.owner_id AND C.pid = P.id AND T.hometown = 'Sangnok City' AND EXISTS(
  SELECT *
  FROM Trainer AS Tr, CatchedPokemon AS Ca
  WHERE Tr.id = Ca.owner_id AND Tr.hometown = 'Brown City'
  AND Ca.pid = C.pid)
ORDER BY P.name;