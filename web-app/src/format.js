// Small formatting helpers shared by all pages.

// 1234.5 -> "₹1,234.50" (Indian digit grouping)
export function money(amount) {
  return '₹' + Number(amount).toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 });
}

// Short form for chart axes: 125000 -> "₹1.3L", 4500 -> "₹4.5K"
export function shortMoney(amount) {
  const a = Math.abs(amount);
  if (a >= 100000) return `₹${(amount / 100000).toFixed(1)}L`;
  if (a >= 1000) return `₹${(amount / 1000).toFixed(1)}K`;
  return `₹${Math.round(amount)}`;
}

// The date picker uses "2026-11-15"; the C++ program uses "15-11-2026".
export function toAppDate(isoDate) {
  const [year, month, day] = isoDate.split('-');
  return `${day}-${month}-${year}`;
}

// "15-11-2026" -> 20261115, so dates can be compared/sorted (same idea as Transaction::getDateKey).
export function dateKey(appDate) {
  const [day, month, year] = appDate.split('-');
  return Number(year + month + day);
}

export function todayIso() {
  const now = new Date();
  now.setMinutes(now.getMinutes() - now.getTimezoneOffset());
  return now.toISOString().slice(0, 10);
}

const MONTHS = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
export const MONTH_NAMES = ['January', 'February', 'March', 'April', 'May', 'June', 'July',
  'August', 'September', 'October', 'November', 'December'];

// month 11, year 2026 -> "Nov 2026"
export function monthLabel(month, year) {
  return `${MONTHS[month - 1]} ${year}`;
}
