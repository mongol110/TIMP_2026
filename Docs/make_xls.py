import openpyxl
base = r"C:\Dev\1231234\TIMP_AES_AUDIO\Docs"

wb = openpyxl.Workbook()
ws = wb.active
ws.title = "test-plan"
ws.append(["ID", "Модуль", "Шаги", "Ожидаемый результат", "Статус"])
rows = [
 ["TP-01", "AUTH", "REGISTER|u1|p1", "ok: registered", "pass"],
 ["TP-02", "AUTH", "REGISTER|u1|p1 повторно", "error: user already exists", "pass"],
 ["TP-03", "AUTH", "AUTH|u1|p1", "ok: authenticated", "pass"],
 ["TP-04", "AUTH", "SHA1|hi без AUTH", "error: not authenticated", "pass"],
 ["TP-05", "SHA1", "SHA1|hello после AUTH", "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d", "pass"],
 ["TP-06", "AES", "AES_ENCRYPT|k|Hi -> AES_DECRYPT", "round-trip == Hi", "pass"],
 ["TP-07", "Newton", "NEWTON|1.5|1e-9", "~1.5213797", "pass"],
 ["TP-08", "Newton", "NEWTON|1.5|100", "error: bad eps", "pass"],
 ["TP-09", "Audio", "EMBED wav+msg -> EXTRACT", "msg совпало", "pass"],
 ["TP-10", "Audio", "EMBED слишком длинное", "error: message too long", "pass"],
 ["TP-11", "Multi", "2 клиента параллельно", "оба получили ответы", "pass"],
 ["TP-12", "DB", "перезапуск, SELECT logs", "логи на месте", "pass"],
]
for r in rows: ws.append(r)
for c in ws.columns:
    ws.column_dimensions[c[0].column_letter].width = 32
wb.save(base + r"\test-plan.xls")

wb2 = openpyxl.Workbook()
ws2 = wb2.active
ws2.title = "test-cases"
ws2.append(["ID", "TestCase", "Шаги", "Баг/дефект", "Статус"])
cases = [
 ["TC-01", "SHA1 пустой", "SHA1|", "—", "pass"],
 ["TC-02", "AES чужой ключ", "DECRYPT другим ключом", "error: decryption failed", "pass"],
 ["TC-03", "Newton df=0", "x0=0.577...", "error: derivative too small", "pass"],
 ["TC-04", "WAV битый", "EXTRACT не-WAV", "error: bad wav", "pass"],
]
for r in cases: ws2.append(r)
for c in ws2.columns:
    ws2.column_dimensions[c[0].column_letter].width = 32
wb2.save(base + r"\test-cases.xls")
print("xls ok")
