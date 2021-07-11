SELECT name
FROM Pokemon AS P
WHERE P.id <> ALL(
  SELECT pid
  FROM CatchedPokemon)
ORDER BY name;