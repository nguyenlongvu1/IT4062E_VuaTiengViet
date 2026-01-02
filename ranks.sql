
DELETE FROM Ranks;

DELETE FROM sqlite_sequence WHERE name='Ranks';

INSERT INTO Ranks (rank_name, min_point, max_point) VALUES 
('Mù chữ', 0, 100),
('Biết chữ sương sương', 101, 200),
('Ngôn từ cấp 1', 201, 500),
('Đủ đậu cấp 3', 501, 800),
('Thủ khoa khối D', 801, 1200),
('Thánh Chém gió', 1201, 1600),
('Bậc Thầy Văn Phong', 1601, 2000),
('Đế Vương Ngôn Ngữ', 2001, 2000000000); 

SELECT * FROM Ranks;