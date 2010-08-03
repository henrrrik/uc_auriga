<?php
// $Id: uc_auriga.pages.inc,v 1.2 2009/04/05 20:41:29 henrrrik Exp $

/**
 * @file
 * auriga menu items.
 *
 */

function uc_auriga_complete($cart_id = 0) {

  $order = uc_order_load(intval($_GET['Customer_refno']));

  if ($order === FALSE || uc_order_status_data($order->order_status, 'state') != 'in_checkout') {
    print uc_order_status_data($order->order_status, 'state');
    print t('An error has occurred during payment.  Please contact us to ensure your order has submitted.');
    exit();
  }

  if (drupal_strtolower($_SESSION['MAC_'. $order->order_id]) != drupal_strtolower($_GET['MAC'])) {
    uc_order_comment_save($order->order_id, 0, t('Attempted unverified Auriga completion for this order.'), 'admin');
  }
  unset($_SESSION['MAC_'. $order->order_id]);

  if ($_GET['Status_code'] == '0') {
    $comment = t('Paid by !type, Auriga transaction ID #!order.', array('!type' => $_GET['Card_type'], '!order' => check_plain($_GET['Transaction_id'])));
    foreach ($order->line_items as $item) {
      $amount = $amount + $item['amount'];
    }
    uc_payment_enter($order->order_id, 'auriga', $amount, 0, NULL, $comment);
  }
  else {
    drupal_set_message(t('The payment could not be completed.  Please verify your payment details and try again.'));
    drupal_goto('<front>');
  }

  // Empty that cart...
  uc_cart_empty($cart_id);

  // Save changes to order without it's completion (it will be on finalization step)
  uc_order_save($order);

  $url = 'cart/auriga/finalize/'. $order->order_id;

  // Javascript redirect on the finalization page.
  $output = '<script type="text/javascript">window.location = "'. url($url, array('absolute' => TRUE)) .'";</script>';

  // Text link for users without Javascript enabled.
  $output .= l(t('Click to complete checkout.'), $url, array('absolute' => TRUE));

  print $output;
  exit();
}

function uc_auriga_finalize() {
  $order = uc_order_load(arg(3));

  // Add a comment to let sales team know this came in through the site.
  uc_order_comment_save($order->order_id, 0, t('Order created through website.'), 'admin');

  $output = uc_cart_complete_sale($order, variable_get('uc_new_customer_login', FALSE));

  $page = variable_get('uc_cart_checkout_complete_page', '');

  if (!empty($page)) {
    drupal_goto($page);
  }

  return $output;
}

