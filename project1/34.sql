SELECT P.name, C.level, C.nickname
FROM Gym AS G, CatchedPokemon AS C, Pokemon AS P
WHERE G.leader_id = C.owner_id AND C.pid = P.id AND nickname LIKE 'A%'
ORDER BY P.name DESC, C.nickname DESC;