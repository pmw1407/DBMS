SELECT T.name
FROM Pokemon AS P, Evolution AS E, CatchedPokemon AS C, Trainer AS T
WHERE P.id = E.before_id AND NOT EXISTS(
  SELECT before_id
  FROM Evolution
  WHERE E.after_id = before_id)
  AND P.id = C.pid AND T.id = C.owner_id
ORDER BY T.name;