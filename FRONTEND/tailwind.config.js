/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{vue,js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        // Apple-inspired colors
        'apple-blue': '#007AFF',
        'apple-gray': '#8E8E93',
        'apple-light-gray': '#F2F2F7',
        'apple-dark-gray': '#1C1C1E',
        'apple-green': '#34C759',
        'apple-red': '#FF3B30',
        'apple-orange': '#FF9500',
        'apple-yellow': '#FFCC00',
        'apple-purple': '#AF52DE',
        'apple-pink': '#FF2782',
        'apple-teal': '#5AC8FA',
        'apple-indigo': '#5856D6',
        // Telegram-inspired colors
        'telegram-blue': '#2AABEE',
        'telegram-dark-blue': '#229ED9',
        'telegram-green': '#00C853',
        'telegram-red': '#E53935',
        'telegram-orange': '#FF6F00',
        'telegram-purple': '#9C27B0',
        'telegram-pink': '#E91E63',
        'telegram-teal': '#00BCD4',
        'telegram-indigo': '#3F51B5',
        'telegram-gray': '#6D6D6D',
        'telegram-light-gray': '#F5F5F5',
        'telegram-dark': '#212121',
      },
      fontFamily: {
        sans: ['-apple-system', 'BlinkMacSystemFont', 'Segoe UI', 'Roboto', 'Helvetica Neue', 'Arial', 'sans-serif'],
      },
      borderRadius: {
        'xl': '1rem',
        '2xl': '1.5rem',
        '3xl': '2rem',
      },
      boxShadow: {
        'apple': '0 4px 20px rgba(0, 0, 0, 0.1)',
        'telegram': '0 2px 10px rgba(0, 0, 0, 0.1)',
      },
      animation: {
        'fade-in': 'fadeIn 0.3s ease-in-out',
        'slide-up': 'slideUp 0.3s ease-in-out',
        'bounce-in': 'bounceIn 0.3s ease-in-out',
      },
      keyframes: {
        fadeIn: {
          '0%': { opacity: '0' },
          '100%': { opacity: '1' },
        },
        slideUp: {
          '0%': { transform: 'translateY(20px)', opacity: '0' },
          '100%': { transform: 'translateY(0)', opacity: '1' },
        },
        bounceIn: {
          '0%': { transform: 'scale(0.9)', opacity: '0' },
          '50%': { transform: 'scale(1.05)' },
          '100%': { transform: 'scale(1)', opacity: '1' },
        },
      },
    },
  },
  plugins: [
    require('@tailwindcss/typography'),
  ],
  darkMode: 'class',
}
