// Convert epoch time to what's needed for program to function
// Please note this is made to work with NTPClient.h
// https://github.com/arduino-libraries/NTPClient

const unsigned long seconds_to_days = 60 * 60 * 24;
const unsigned long seconds_to_years_with_leap = seconds_to_days * 365.2422;

int year(unsigned long epoch_time) {
  return epoch_time / (seconds_to_years_with_leap) + 1970;
}

int day_of_year(unsigned long epoch_time) {
  return (epoch_time / seconds_to_days) - (epoch_time / seconds_to_years_with_leap) * 365.2422 + 1;
}

int day_of_week(unsigned long epoch_time) { // Thursday is 0
  return (epoch_time / seconds_to_days) % 7;
}

int day_of_month(unsigned long epoch_time) {
  int days = day_of_year(epoch_time);
  int y = year(epoch_time);
  int feb_days = 28;
  if (days <= 31) return days;
  days -= 31;
  if ((y % 100 != 0 && y % 4 == 0) || (y % 100 == 0 && y % 400 == 0)) feb_days = 29;
  if (days <= feb_days) return days;
  days -= feb_days;
  if (days <= 31) return days;
  days -= 31;
  if (days <= 30) return days;
  days -= 30;
  if (days <= 31) return days;
  days -= 31;
  if (days <= 30) return days;
  days -= 30;
  if (days <= 31) return days;
  days -= 31;
  if (days <= 31) return days;
  days -= 31;
  if (days <= 30) return days;
  days -= 30;
  if (days <= 31) return days;
  days -= 31;
  if (days <= 30) return days;
  days -= 30;
  if (days <= 31) return days;
  else return days;
}

int month(unsigned long epoch_time) {
  int days = day_of_year(epoch_time);
  int y = year(epoch_time);
  int feb_days = 28;
  if (days <= 31) return 1;
  days -= 31;
  if (y % 100 != 0 && y % 4 == 0 || y % 100 == 0 && y % 400 == 0) feb_days = 29;
  if (days <= feb_days) return 2;
  days -= feb_days;
  if (days <= 31) return 3;
  days -= 31;
  if (days <= 30) return 4;
  days -= 30;
  if (days <= 31) return 5;
  days -= 31;
  if (days <= 30) return 6;
  days -= 30;
  if (days <= 31) return 7;
  days -= 31;
  if (days <= 31) return 8;
  days -= 31;
  if (days <= 30) return 9;
  days -= 30;
  if (days <= 31) return 10;
  days -= 31;
  if (days <= 30) return 11;
  days -= 30;
  if (days <= 31) return 12;
  else return 0;
}

bool is_DST(unsigned long epoch_time) {
  int dw = day_of_week(epoch_time);
  int dm = day_of_month(epoch_time);
  int m = month(epoch_time);

  // if middle month or after second Sunday in March or before first Sunday in November
  if (
    (3 < m && m < 11) ||
    (m == 3 && (dm > 7 && ((dw >= 3 && dw - dm <=  2) || (dw < 3 && dw - dm <= -5)))) ||
    (m == 11 &&           ((dw < 3 && dw - dm >= -4) || (dw > 3 && dw - dm >=  3)))
  ) return true;
  else return false;
}
