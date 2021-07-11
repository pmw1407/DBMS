SELECT F.name
FROM Evolution AS E, Pokemon AS P, Pokemon AS F
WHERE P.id = E.before_id AND NOT EXISTS(
  SELECT *
  FROM Evolution
  WHERE before_id = E.after_id)
  AND F.id = E.after_id
ORDER BY F.name;