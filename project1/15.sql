SELECT Result.owner_id, Result.num
FROM (
  SELECT owner_id, COUNT(*) AS num
  FROM CatchedPokemon AS C
  GROUP BY owner_id) AS Result
WHERE Result.num IN(
  SELECT MAX(numOfPokemon)
  FROM (
    SELECT owner_id, COUNT(*) AS numOfPokemon
    FROM CatchedPokemon AS C
    GROUP BY owner_id) AS result)
ORDER BY Result.owner_id;