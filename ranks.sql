-- 1. Xóa sạch dữ liệu Rank cũ
DELETE FROM Ranks;

-- 2. Reset ID về 1
DELETE FROM sqlite_sequence WHERE name='Ranks';

-- 3. Chèn dữ liệu Rank mới
INSERT INTO Ranks (rank_name, min_point, max_point) VALUES 
('Mù chữ', 0, 100),
('Biết chữ sương sương', 101, 200),
('Ngôn từ cấp 1', 201, 500),
('Đủ đậu cấp 3', 501, 800),
('Thủ khoa khối D', 801, 1200),
('Thánh Chém gió', 1201, 1600),
('Bậc Thầy Văn Phong', 1601, 2000),
('Đế Vương Ngôn Ngữ', 2001, 2000000000); -- Max là 2 tỷ

-- Kiểm tra lại xem đã chuẩn chưa
SELECT * FROM Ranks;