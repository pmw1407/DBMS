SELECT T.name
FROM Trainer as T
WHERE T.id <> ALL(
  SELECT leader_id
  FROM Gym)
ORDER BY name;