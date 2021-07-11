SELECT name
FROM CatchedPokemon AS C, Pokemon AS P
WHERE C.pid = P.id AND nickname LIKE '% %'
ORDER BY name DESC;