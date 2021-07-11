SELECT hometown, AVG(C.level) AS average
FROM Trainer AS T, CatchedPokemon as C
WHERE T.id = C.owner_id
GROUP BY hometown
ORDER BY average;