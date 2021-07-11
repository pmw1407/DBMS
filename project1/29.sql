SELECT COUNT(type) 
FROM Pokemon AS P, CatchedPokemon AS C
WHERE P.id = C.pid
GROUP BY type
ORDER BY type;