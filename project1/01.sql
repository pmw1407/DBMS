SELECT T.name
FROM CatchedPokemon as C, Trainer as T
WHERE T.id = C.owner_id
GROUP BY owner_id
HAVING COUNT(owner_id) > 2
ORDER BY COUNT(owner_id) DESC;