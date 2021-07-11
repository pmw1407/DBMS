SELECT Trainer.hometown, nickname
FROM(SELECT hometown, MAX(C.level) AS maxlv
FROM CatchedPokemon AS C, Trainer AS T
WHERE C.owner_id = T.id
GROUP BY hometown) AS result, Trainer, CatchedPokemon
WHERE Trainer.id = CatchedPokemon.owner_id AND result.hometown = Trainer.hometown AND result.maxlv = CatchedPokemon.level
ORDER BY Trainer.hometown;
