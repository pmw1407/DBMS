SELECT AVG(level)
FROM Gym AS G, CatchedPokemon as C
WHERE G.leader_id = C.owner_id