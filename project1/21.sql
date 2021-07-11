SELECT name, COUNT(*)
FROM Trainer as T, Gym AS G, CatchedPokemon as C
WHERE G.leader_id = C.owner_id AND T.id = G.leader_id AND T.id = C.owner_id
GROUP BY G.leader_id
ORDER BY T.name;