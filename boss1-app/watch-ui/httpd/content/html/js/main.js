/**
 * @file main.js
 * @brief Main JavaScript for captive portal
 * @author BOSS1
 * @version 1.0
 * @date 2024
 */

(function() {
    'use strict';

    window.WatchUI = window.WatchUI || {};

    WatchUI.api = {
        baseUrl: '/cgi-bin/cgi',

        async request(endpoint, data) {
            const url = this.baseUrl + endpoint;
            const response = await fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data || {}),
            });
            return response.json();
        },

        async login(username, password) {
            return this.request('/login', { username, password });
        },

        async getStatus() {
            return this.request('/status', {});
        },

        async logout() {
            return this.request('/logout', {});
        }
    };

    WatchUI.utils = {
        showError(message) {
            const errorEl = document.getElementById('error-message');
            if (errorEl) {
                errorEl.textContent = message;
                errorEl.style.display = 'block';
            }
        },

        hideError() {
            const errorEl = document.getElementById('error-message');
            if (errorEl) {
                errorEl.style.display = 'none';
            }
        },

        showLoading() {
            const btn = document.querySelector('button[type="submit"]');
            if (btn) {
                btn.disabled = true;
                btn.dataset.originalText = btn.textContent;
                btn.textContent = 'Loading...';
            }
        },

        hideLoading() {
            const btn = document.querySelector('button[type="submit"]');
            if (btn && btn.dataset.originalText) {
                btn.disabled = false;
                btn.textContent = btn.dataset.originalText;
            }
        }
    };

    document.addEventListener('DOMContentLoaded', function() {
        console.log('Watch UI Portal Loaded');
    });

})();
